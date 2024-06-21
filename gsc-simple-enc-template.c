/*
	MIT License

	Copyright (c) 2024 Mohannad Shehadeh

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * This program provides a template for implementing the
 * simplest possible generalized staircase code encoder
 * and prints a visualization of the encoding process.
 * Note that much more memory-efficient encoding
 * is possible than what is modeled here.
 */

/*
 * Make sure to define M, MEMORY, and GOLOMB correctly!
 * GOLOMB must be an array of M+1 distinct increasing
 * integers starting at 0.
 * MEMORY must be its maximum element.
 */
#define M 2
static const int GOLOMB[M+1] = {0, 1, 3};
#define MEMORY 3
#define S 3

// forward intra-block/tile permutation
int64_t pi_row(int64_t i, int64_t j, int64_t k) {
	return k == 0 ? i : j;
}
int64_t pi_col(int64_t i, int64_t j, int64_t k) {
	return k == 0 ? j : (i+(k-1)*j)%S;
}

// inverse intra-block/tile permutation (not needed for encoding)
// int64_t pi_inv_row(int64_t i, int64_t j, int64_t k) {
//  return k == 0 ? i : ((S-(k-1))*i + j)%S;
// }
// int64_t pi_inv_col(int64_t i, int64_t j, int64_t k) {
//  return k == 0 ? j : i;
// }

/*
 * Encoder circular buffer is buffer of past Q number of
 * 1-by-S rows where Q = (1+MEMORY)*S-1.
 * We do modulo-Q indexing of rows.
 */
#define Q ((1+MEMORY)*S-1)
uint32_t enc_buffer[Q][S];
int32_t NewestRow; // index of newest row

//input i: index of row being encoded: i = 0,1,...,S-1
//input j: position in prepend data to populate: j = 0,1,...,M*S-1
//output past_row_lookback: how many rows to look back in the encoder buffer: 1,2,...,(1+MEMORY)*S-1
//output past_col: position within past row to copy from: 0,1,...,S-1
int32_t past_row_lookback(int32_t i, int32_t j) {
	int32_t perm = M-j/S; // M, M-1, ..., 1
	int32_t j_block = j%S; // 0, 1, ..., S-1
	return GOLOMB[perm]*S - pi_row(i,j_block,perm) + i;
}
int32_t past_col(int32_t i, int32_t j) {
	int32_t perm = M-j/S; // M, M-1, ..., 1
	int32_t j_block = j%S; // 0, 1, ..., S-1
	return pi_col(i,j_block,perm);
}

int main() {
	// initialize buffer
	memset(enc_buffer, 0, Q*S*sizeof(uint32_t));
	NewestRow = 0;

	int32_t data_label = 1;

	for (int count = 0; count < MEMORY+3; count++) {

		for (int i = 0; i < S; i++) { // row i encoding
			// prepended part
			for (int j = 0; j < M*S; j++) {
				//populate the first MS bits of component code encoder input with this data
				//from the enc_buffer
				printf("%4d", enc_buffer[(NewestRow+1-past_row_lookback(i,j)+Q)%Q][past_col(i,j)]);
			}
			// appended part (info, parity)
			if (++NewestRow >= Q) NewestRow = 0; // mod Q increment
			for (int j = 0; j < S; j++) {
				// populate the remaining S-r bits of component encoder input,
				// encode to get a full row of S bits, and add it to the encoder
				// buffer. Here, we overwrite the previous oldest block.
				enc_buffer[NewestRow][j] = data_label;
				printf("%4d", enc_buffer[NewestRow][j]);
				data_label++;
			}
			printf("\n");
		}

	}
	return 0;
}
