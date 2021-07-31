package rlottie

// #cgo LDFLAGS: -lrlottie -lstdc++ -lm
// #include "rlottie_capi.h"
// void lottie_configure_model_cache_size(size_t cacheSize);
import "C"
import "unsafe"
// Compile flags for statically linked
// git clone https://github.com/Samsung/rlottie
// cd rlottie
// cmake -DBUILD_SHARED_LIBS=OFF -DLOTTIE_MODULE=0 -DCMAKE_BUILD_TYPE=Release .
type Lottie_Animation struct {
    Ptr *C.Lottie_Animation
}

var disabled_cache = false

func LottieAnimationFromData(s string) Lottie_Animation {
    if !disabled_cache {
        disabled_cache = true
        C.lottie_configure_model_cache_size(0)
    }
    var animation Lottie_Animation
    animation.Ptr = C.lottie_animation_from_data(C.CString(s), C.CString(""), C.CString(""))
    return animation
}

func LottieAnimationDestroy(animation Lottie_Animation) {
    C.lottie_animation_destroy(animation.Ptr)
}

func LottieAnimationGetSize(animation Lottie_Animation) (uint, uint) {
    var width C.size_t
    var height C.size_t
    C.lottie_animation_get_size(animation.Ptr, &width, &height)
    return uint(width), uint(height)
}

func LottieAnimationGetTotalframe(animation Lottie_Animation) uint {
    return uint(C.lottie_animation_get_totalframe(animation.Ptr))
}

func LottieAnimationGetFramerate(animation Lottie_Animation) float64 {
    return float64(C.lottie_animation_get_framerate(animation.Ptr))
}

func LottieAnimationGetFrameAtPos(animation Lottie_Animation, pos float32) uint {
    return uint(C.lottie_animation_get_frame_at_pos(animation.Ptr, C.float(pos)))
}

func LottieAnimationGetDuration(animation Lottie_Animation) float64 {
    return float64(C.lottie_animation_get_duration(animation.Ptr))
}

func LottieAnimationRender(animation Lottie_Animation, frame_num uint, buffer []byte, width uint, height uint, bytes_per_line uint) {
    var ptr *C.uint32_t = (*C.uint32_t)(unsafe.Pointer(&buffer[0]));
    C.lottie_animation_render(animation.Ptr, C.size_t(frame_num), ptr, C.size_t(width), C.size_t(height), C.size_t(bytes_per_line))
}
