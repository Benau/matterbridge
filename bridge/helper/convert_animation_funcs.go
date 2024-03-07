//go:build cgolibs

package helper

/*

#include <stdint.h>
#include <stdlib.h>

int writer_cgo(size_t buffer_length, const uint8_t *buffer, void *user_data)
{
    return writerGo(buffer_length, buffer, user_data);
}
*/
import "C"
