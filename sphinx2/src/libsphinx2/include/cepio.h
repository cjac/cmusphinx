/* Read and write of cepstral vectors from disk */

#ifndef _CEPIO_H_
#define _CEPIO_H_

int32 cep_read_bin (float32 **buf, int32 *len, char const *file);
int32 cep_write_bin(char const *file, float32 *buf, int32 len);

#endif /* _CEPIO_H_ */
