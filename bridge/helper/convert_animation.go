//go:build cgolibs

package helper

// #cgo LDFLAGS: -lavformat -lavcodec -lavutil -lswscale -lgifski -O0 -g
// #include <animation_to_gif.h>
// #include <string.h>
// int writer_cgo(size_t buffer_length, const uint8_t *buffer, void *user_data);
import "C"
import "unsafe"
//import "io/ioutil"

var output []byte

//export writerGo
func writerGo(size C.size_t, buffer unsafe.Pointer, user_data unsafe.Pointer) int {
	if size == 0 {
		return 0
	}
	output = append(output, (*[1 << 30]byte)(buffer)[:size]...)
	return 0
}

func ConvertAnimation(name *string, data *[]byte) {
	output = make([]byte, 0)
	var ptr *C.uint8_t = (*C.uint8_t)(unsafe.Pointer(&(*data)[0]));
	if C.animationToGif(ptr, C.int(len(*data)), (C.writeCallback)(unsafe.Pointer(C.writer_cgo))) == 0 {
		*data = output
		*name = *name + ".gif"
		//ioutil.WriteFile(*name, *data, 0644)
	}
}
