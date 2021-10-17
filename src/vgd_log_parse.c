/*
Copyright (c) 2021 Avinash ("Avi") Teek Singh. All Rights Reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1.
Redistribution of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.

2.
Redistribution in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

3.
Neither the name of the copyright holder nor the names of its contributors may
be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

YOU ACKNOWLEDGE THAT THIS SOFTWARE IS NOT DESIGNED, LICENSED OR INTENDED FOR
USE IN THE DESIGN, CONSTRUCTION, OPERATION OR MAINTENANCE OF ANY MILITARY
AND/OR NUCLEAR FACILITY FOR PURPOSES OF LIFE SUPPORT OR OTHER MISSION CRITICAL
APPLICATIONS WHERE HUMAN LIFE OR PROPERTY MAY BE AT STAKE.
*/


#include "vgd_log_parse.h"


/*
** Internal VGA open log function. Returns an input struct with
** a file descriptor on success, NULL on struct allocation fail.
*/
VGD_input *VGD_open(const char *fname) {

	VGD_input *ret = NULL;
	FILE *vgd_fd;

	// allocate a VGD input struct as needed
	if((vgd_fd = fopen(fname,"r"))) {
		ret = (VGD_input *)malloc(sizeof(VGD_input));
		ret -> vgd_fd = vgd_fd;
		__VGD_count_logs(ret);
		if(ret -> log_count == 0) {
			fprintf(stderr, "VGD_open() : An error occured. No logs registered in __VGD_count_logs().\n");
			fprintf(stderr, "	Problematic file in question: %s.\n", fname);
			fprintf(stderr, "	Check for errors from __VGD_count_logs() and __VGD_store_log_offsets().\n");
		}
	}

	return(ret);

}


/*
** Internal VGA close log function. Returns 0 on success and the FILE *
** that could not be closed on fail. Also frees the input struct.
*/
FILE *VGD_close(VGD_input **vgd_input) {

	// zero/null out members
	int i;

	unsigned int log_count = (*vgd_input) -> log_count;

	for(i = 0; i < log_count; i++) { (*vgd_input) -> log_offset[i] = 0; }
	free((*vgd_input) -> log_offset);	
	(*vgd_input) -> log_offset = NULL;
	(*vgd_input) -> current_log_offset = 0;
	(*vgd_input) -> log_count = 0;
	for(i = 0; i < 4; i++) { (*vgd_input) -> line_data[i] = 0; }

	// close the log file
	unsigned long long int err;
	FILE *ret;

	// file descriptor no longer open; remove from struct
	if(!(err = fclose((*vgd_input) -> vgd_fd))) {
		ret = NULL;
	// fclose returned an error; print and return value of file descriptor
	} else {
		ret = (*vgd_input) -> vgd_fd;
		fprintf(stderr, "VGD_close() : File close error. fclose(FILE *) returned %llu.\n", err);
		fprintf(stderr, "	FILE * in question is %p.\n", ret);
	}

	// clear the file descriptor from the struct
	(*vgd_input) -> vgd_fd = NULL;
	free(*vgd_input);
	*vgd_input = NULL;

	// returns NULL if file successfully closed; FILE * if otherwise
	return(ret);
	
}


/*
**	Indicates the number of logs found in file given by the VGD
**	input struct's file descriptor. Stores the number of logs found
**	to the input struct's log count member. Calls __VGD_store_log_offsets
**	to store the location of the start of each log in the file.
*/
void __VGD_count_logs(VGD_input *vgd_input) {

	unsigned char line_data[4] = {0, 0, 0, 0}; 
	FILE *vgd_fd = vgd_input -> vgd_fd;
	unsigned int log_count = 0;
	unsigned char data;

	// reset the position in the file to the start
	int err;
	if((err = fseek(vgd_fd, SEEK_SET, 0))) {
		fprintf(stderr, "__VGD_count_logs() : Could not count logs in file. fseek(vgd_fd, SEEK_SET, 0) returned %d.\n", err);
		goto VGD_count_logs_feof;
	}

	// read until end of file is reached
	while(1) {

		int i;

		// load 4 chars into the buffer
		for(i = 0; i < 4; i++) {
			data = fgetc(vgd_fd);
			if(!feof(vgd_fd)) {
				line_data[i] = data;
			} else {
				goto VGD_count_logs_feof;
			}
		}

		// log delimiter detected
		if(
			line_data[0] == 'V' &&
			line_data[1] == 'G' &&
			line_data[2] == 0xFF &&
			line_data[3] == 0xFF
		) {
			log_count++;
		}

	}

	VGD_count_logs_feof:
	
	if(!err) {
		vgd_input -> log_count = log_count;
		__VGD_store_log_offsets(vgd_input);
	} else {
		vgd_input -> log_count = 0;
	}

}


/*
**	Stores the character offset of each log to the log_offset array member
**	of the input struct.
*/
void __VGD_store_log_offsets(VGD_input *vgd_input) {

	unsigned char line_data[4] = {0, 0, 0, 0}; 
	FILE *vgd_fd = vgd_input -> vgd_fd;
	unsigned int log_count = 0;
	unsigned char data;

	// reset the position in the file to the start
	int err;
	if((err = fseek(vgd_fd, 0, SEEK_SET))) {
		fprintf(stderr, "__VGD_store_log_offsets() : Could not count logs in VGD. fseek(vgd_fd, SEEK_SET, 0) returned %d.\n", err);
		goto VGD_store_log_offsets_feof;
	}
	
	unsigned long long int *log_offset = (unsigned long long int *)malloc((vgd_input -> log_count) * sizeof(unsigned long long int));

	unsigned long long int i = 0;

	// read until end of file is reached
	while(1) {

		if(log_count == vgd_input -> log_count) { goto VGD_store_log_offsets_feof; }

		int j = 0;
		// load 4 chars into the buffer
		for(j = 0; j < 4; j++) {
			data = fgetc(vgd_fd);
			if(!feof(vgd_fd)) {
				line_data[j] = data;
			} else {
				goto VGD_store_log_offsets_feof;
			}
		}

		i += 4;

		// log delimiter detected
		if(
			line_data[0] == 'V' &&
			line_data[1] == 'G' &&
			line_data[2] == 0xFF &&
			line_data[3] == 0xFF
		) {
			log_offset[log_count] = i;
			log_count++;
		}

	}

	VGD_store_log_offsets_feof:
	
	if(!err) {
		vgd_input -> log_offset = log_offset;
	} else {
		vgd_input -> log_count = 0;
	}

}


/*
**	Sets the offset of the given log. Returns 0 on success and a nonzero value
**	on fail. A return of 1 may indicate an invalid log number was selected.
**	Otherwise, the value returned represents the return of fseek().
*/
unsigned int VGD_select_log(VGD_input *vgd_input, unsigned int log_nbr) {

	unsigned int ret = 0;
	int i;
	for(i = 0; i < 4; i++) {
		vgd_input -> line_data[i] = 0;
	}

	if(log_nbr > vgd_input -> log_count) {
		fprintf(
			stderr,
			"VGD_select_log() : Bad log number supplied.\n"
		);
		fprintf(
			stderr,
			"	File contains %d logs.\n",
			vgd_input -> log_count
		);
		ret = 1;
	}
	
	if(log_nbr == 0) {
		fprintf(
			stderr,
			"VGD_select_log() : Bad log number supplied.\n"
		);
		fprintf(
			stderr,
			"	Log number must be greater than 0.\n"
		);
		ret = 1;
	}

	if(ret) { goto VGD_select_log_ret; }

	// set the offset
	vgd_input -> current_log_offset = vgd_input -> log_offset[log_nbr - 1];

	// attempt to update the current offset in the file
	if((ret = fseek(vgd_input -> vgd_fd, vgd_input -> current_log_offset, SEEK_SET))) {
		fprintf(
			stderr,
			"VGD_select_log() : Could not seek position. fseek() == %d\n",
			ret
		);
	}

	VGD_select_log_ret:
	return(ret);

}


/*
**	Reads from the selected log line by line. A call to VGD_select_log()
**	must be made before using this function. This will read until it
**	hits an EOF or reads in a 4-byte {'V', 'G', 0xFF, 0xFF} pattern.
*/
unsigned int VGD_read_line(VGD_input *vgd_input) {

	unsigned int ret = 1;
	unsigned char data = 0;
	FILE *vgd_fd = vgd_input -> vgd_fd;
	unsigned char *line_data = vgd_input -> line_data;
	
	// load 4-character data into the buffer
	int i;
	for(i = 0; i < 4 && ret; i++) {
		data = fgetc(vgd_fd);
		if(!feof(vgd_fd)) {
			// load character into buffer unless EOF is reached
			line_data[i] = data;
		} else {
			// line ended prematurely; do not read any further
			ret = 0;
		}
	}
	
	// check that a log boundary (log_dat = {'V','G',0xFF,0xFF})
	// hasn't been read in
	if(ret) {
		if(
			line_data[0] == 'V' &&
			line_data[1] == 'G' &&
			line_data[2] == 0xFF &&
			line_data[3] == 0xFF
		) {
			ret = 0;
		}
	}
	
	if(ret) {
		vgd_input -> current_log_offset++;
	}
	
	return(ret);

}