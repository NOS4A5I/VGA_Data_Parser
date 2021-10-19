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


#ifndef __VGD_LOG_PARSE__
#define __VGD_LOG_PARSE__

#include <stdio.h>
#include <stdlib.h>


/*
** A struct that holds information about the opened log.
*/
typedef struct {
	FILE *vgd_fd;
	unsigned char line_data[4];
	unsigned int log_count;
	unsigned long long int *log_offset;
	unsigned long long int current_log_offset;
} VGD_input;


/*
** Internal VGA open log function. Returns an input struct with
** a file descriptor on success, NULL on struct allocation fail.
*/
VGD_input *VGD_open(const char *fname);


/*
** Internal VGA close log function. Returns 0 on success and the FILE *
** that could not be closed on fail. Also frees the input struct.
*/
FILE *VGD_close(VGD_input **vgd_input);


/*
**	Indicates the number of logs found in file given by the VGD
**	input struct's file descriptor. Stores the number of logs found
**	to the input struct's log count member.
*/
void __VGD_count_logs(VGD_input *vgd_input);


/*
**	Stores the character offset of each log to the log offset array member
**	of the input struct.
*/
void __VGD_store_log_offsets(VGD_input *vgd_input);


/*
**	Sets the offset of the given log. Returns 0 on success and a nonzero value
**	on fail. A return of 1 may indicate an invalid log number was selected.
**	Otherwise, the value returned represents the return of fseek().
*/
unsigned int VGD_select_log(VGD_input *vgd_input, unsigned int log_nbr);


/*
**	Reads from the selected log line by line. A call to VGD_select_log()
**	must be made before using this function. This will read until it
**	hits an EOF or reads in a 4-byte {'V', 'G', 0xFF, 0xFF} pattern.
*/
unsigned int VGD_read_line(VGD_input *vgd_input);


#endif // __VGD_LOG_PARSE__