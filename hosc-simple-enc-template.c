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
 * simplest possible higher-order staircase code encoder
 * and prints a visualization of the encoding process.
 * Note that much more memory-efficient encoding
 * is possible than what is modeled here.
 */

/*
 * Make sure to define L, M, SCOPE, and DTS correctly!
 * DTS must be an L x (M+1) array where each row
 * contains distinct increasing integers starting at 0
 * and the rows are sorted in descending order of their
 * respective maximum elements.
 * SCOPE must be the maximum element across all rows.
 */
#define L 2
#define M 2
static const int DTS[L][M+1] = {{0,2,7},{0,3,4}};
#define SCOPE 7
#define T 2

#define LT (L*T)

// forward intra-block/tile permutation
int64_t pi_row(int64_t i, int64_t j, int64_t k) {
	return k == 0 ? i : j;
}
int64_t pi_col(int64_t i, int64_t j, int64_t k) {
	return k == 0 ? j : (i+(k-1)*j)%T;
}

// inverse intra-block/tile permutation (not needed for encoding)
// int64_t pi_inv_row(int64_t i, int64_t j, int64_t k) {
//  return k == 0 ? i : ((T-(k-1))*i + j)%T;
// }
// int64_t pi_inv_col(int64_t i, int64_t j, int64_t k) {
//  return k == 0 ? j : i;
// }

/*
 * Encoder circular buffer is buffer of past Q number of
 * 1-by-(L*T) rows where Q = (1+SCOPE)*T-1.
 * We do modulo-Q indexing of rows.
 */
#define Q ((1+SCOPE)*T-1)
uint32_t enc_buffer[Q][LT];
int32_t NewestRow; // index of newest row

//input i: index of row being encoded: i = 0,1,...,T-1
//input j: position in prepend data to populate: j = 0,1,...,M*L*T-1
//output past_row_lookback: how many rows to look back in the encoder buffer: 1,2,...,(1+SCOPE)*T-1
//output past_col: position within past row to copy from: 0,1,...,L*T-1
int32_t past_row_lookback(int32_t i, int32_t j) {
	int32_t j_rect = j%LT;
	// j/LT = 0,1,...,M-1
	int32_t perm = M-j/LT; // M, M-1, ..., 1
	int32_t j_block = j_rect%T; // 0, 1, ..., T-1
	int32_t class = L-1-j_rect/T; // L-1, L-2, ... , 1, 0
	return DTS[class][perm]*T - pi_row(i,j_block,perm) + i;
}
int32_t past_col(int32_t i, int32_t j) {
	int32_t j_rect = j%LT;
	// j/LT = 0,1,...,M-1
	int32_t perm = M-j/LT; // M, M-1, ..., 1
	int32_t j_block = j_rect%T; // 0, 1, ..., T-1
	int32_t c_class = j_rect/T; // 0,1,...,L-1
	return c_class*T + pi_col(i,j_block,perm);
}

int main() {
	// initialize buffer
	memset(enc_buffer, 0, Q*LT*sizeof(uint32_t));
	NewestRow = 0;

	int32_t data_label = 1;

	for (int count = 0; count < SCOPE+3; count++) {

		for (int i = 0; i < T; i++) { // row i encoding
			// prepended part
			for (int j = 0; j < M*L*T; j++) {
				//populate the first MLT bits of component code encoder input with this data
				//from the enc_buffer
				printf("%4d", enc_buffer[(NewestRow+1-past_row_lookback(i,j)+Q)%Q][past_col(i,j)]);
			}
			// appended part (info, parity)
			if (++NewestRow >= Q) NewestRow = 0; // mod Q increment
			for (int j = 0; j < L*T; j++) {
				// populate the remaining LT-r bits of component encoder input,
				// encode to get a full row of LT bits, and add it to the encoder
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






