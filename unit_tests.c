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


#include <stdio.h>
#include <errno.h>

#include "src/vgd_log_parse.h"

const char *nexistfname = "thisfiledoesnotexist.aaa";
// correct filepaths if executed in project's build folder
const char *fname = "../testfiles/vgd_parse_testfile1.vgd";
const char *misftfname = "../testfiles/vgd_parse_testfile2.vgd";
const char *emptyfname = "../testfiles/empty";

int main(int argc, char *argv[]) {

	printf("\nSTARTING UNIT TEST.\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
	
	unsigned char *line_data;

	printf("\nAttempting to open nonexistent log file %s.\n", nexistfname);
	VGD_input *logf = VGD_open(nexistfname);
	if(logf) {
		printf("VGD input log struct successfully allocated. (BAD)\n");
		exit(EBADF);
	} else {
		printf("Failed to allocate VGD input log struct. (GOOD)\n");
	}

	printf("\nAttempting to open empty log file %s.\n", emptyfname);
	logf = VGD_open(emptyfname);
	if(logf) {
		printf("VGD input log struct successfully allocated. (GOOD)\n");
	} else {
		printf("Failed to allocate VGD input log struct. (BAD)\n");
		exit(EBADF);
	}
	
	int i;
	
	line_data = logf -> line_data;
	
	printf("\nAttempting to select and read logs from an empty file 10 times.\n");
	for(i = 0; i < 10; i++) {
		VGD_select_log(logf, i);
		while(VGD_read_line(logf)) {
			printf("Line:	");
			// print sync signals and RGB values
			printf(
				"HS = %d	VS = %d	DE = %d	",
				(line_data[3] & 128) > 0,
				(line_data[3] & 64) > 0,
				(line_data[3] & 32) > 0
			);
			printf(
				"RGB = (%d,%d,%d)\n",
				line_data[0],
				line_data[1],
				line_data[2]
			);
		}
	}
	
	VGD_close(&logf);	
	

	printf("\nAttempting to open valid log file %s.\n", fname);
	logf = VGD_open(fname);
	if(logf) {
		printf("VGD input log struct successfully allocated. (GOOD)\n");
	} else {
		printf("Failed to allocate VGD input log struct. Check if the input file is valid. (BAD)\n");
		exit(EBADF);
	}

	if(!(logf -> vgd_fd)) {
		printf("Could not open VGD log. Exiting.\n");
		exit(EBADF);
	} else {
		printf("VGD log descriptor %p.\n", logf -> vgd_fd);
	}

	printf("Counted %d logs. Result should be 3.\n",logf -> log_count);
	
	printf("\nLog offsets:\n");

	unsigned int log_count = logf -> log_count;
	for(i = 0; i < log_count; i++) {
		printf("Log %d @ file offset 0x%llx.\n",i+1,logf -> log_offset[i]);
	}
	
	printf("\nAttempting to select an invalid log number (too high).\n");
	VGD_select_log(logf, logf -> log_count + 1);
	printf("\nAttempting to select an invalid log number (too low).\n");
	VGD_select_log(logf, 0);
	
	line_data = logf -> line_data;
	
	for(i = log_count; i > 0; i--) {
		
		printf("\nSelecting log %d.\n", i);
		VGD_select_log(logf, i);
		
		printf("Reading log %d @ offset 0x%llx:\n", i, logf -> current_log_offset);

		while(VGD_read_line(logf)) {
			printf("Line:	");
			// print sync signals and RGB values
			printf(
				"HS = %d	VS = %d	DE = %d	",
				(line_data[3] & 128) > 0,
				(line_data[3] & 64) > 0,
				(line_data[3] & 32) > 0
			);
			printf(
				"RGB = (%d,%d,%d)\n",
				line_data[0],
				line_data[1],
				line_data[2]
			);
		}
		printf("End of log detected.\n");
	}
	
	printf("\nClosing log file.\n");
	VGD_close(&logf);
	
	printf("\nAttempting to open another valid log file %s, but slightly misformatted.\n", misftfname);
	logf = VGD_open(misftfname);
	if(logf) {
		printf("VGD input log struct successfully allocated. (GOOD)\n");
	} else {
		printf("Failed to allocate VGD input log struct. Check if the input file is valid. (BAD)\n");
		exit(EBADF);
	}

	if(!(logf -> vgd_fd)) {
		printf("Could not open VGD log. Exiting.\n");
		exit(EBADF);
	} else {
		printf("VGD log descriptor %p.\n", logf -> vgd_fd);
	}

	printf("Counted %d logs. Result should be 1.\n",logf -> log_count);
	
	printf("\nLog offsets:\n");

	log_count = logf -> log_count;
	for(i = 0; i < log_count; i++) {
		printf("Log %d @ file offset 0x%llx.\n",i+1,logf -> log_offset[i]);
	}
	
	printf("\nAttempting to select an invalid log number (too high).\n");
	VGD_select_log(logf, logf -> log_count + 1);
	printf("\nAttempting to select an invalid log number (too low).\n");
	VGD_select_log(logf, 0);
	
	line_data = logf -> line_data;
	
	for(i = log_count; i > 0; i--) {
		
		printf("\nSelecting log %d.\n", i);
		VGD_select_log(logf, i);
		
		printf("Reading log %d @ offset 0x%llx:\n", i, logf -> current_log_offset);

		while(VGD_read_line(logf)) {
			printf("Line:	");
			// print sync signals and RGB values
			printf(
				"HS = %d	VS = %d	DE = %d	",
				(line_data[3] & 128) > 0,
				(line_data[3] & 64) > 0,
				(line_data[3] & 32) > 0
			);
			printf(
				"RGB = (%d,%d,%d)\n",
				line_data[0],
				line_data[1],
				line_data[2]
			);
		}
		printf("End of log detected.\n");
	}
	
	printf("\nClosing log file.\n");
	VGD_close(&logf);
	
	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("UNIT TEST COMPLETED.\n\n");

	return(0);

}