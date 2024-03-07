#include "animation_to_gif.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include <gifski.h>
#include <stdio.h>

int animationToGif(unsigned char *videoData, int videoSize, writeCallback cb) {
	unsigned char *inbuffer = (unsigned char *)av_malloc(videoSize);
	memcpy(inbuffer, videoData, videoSize);

	AVFormatContext *inputFormatContext = avformat_alloc_context();
	AVIOContext *avioContext = avio_alloc_context(inbuffer, videoSize, 0, NULL, NULL, NULL, NULL);
	inputFormatContext->pb = avioContext;

	if (avformat_open_input(&inputFormatContext, NULL, NULL, NULL) != 0) {
		fprintf(stderr, "Error opening input file\n");
		av_free(avioContext->buffer);
		av_free(avioContext);
		avformat_close_input(&inputFormatContext);
		return -1;
	}

	if (avformat_find_stream_info(inputFormatContext, NULL) < 0) {
		fprintf(stderr, "Error finding stream information\n");
		av_free(avioContext->buffer);
		av_free(avioContext);
		avformat_close_input(&inputFormatContext);
		return -1;
	}

	int videoStreamIndex = -1;
	for (int i = 0; i < inputFormatContext->nb_streams; i++) {
		if (inputFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStreamIndex = i;
			break;
		}
	}

	if (videoStreamIndex == -1) {
		fprintf(stderr, "No video stream found\n");
		av_free(avioContext->buffer);
		av_free(avioContext);
		avformat_close_input(&inputFormatContext);
		return -1;
	}

	AVStream* stream = inputFormatContext->streams[videoStreamIndex];
	AVCodecParameters *codecParameters = stream->codecpar;
	double fps = (double)stream->r_frame_rate.num / stream->r_frame_rate.den;
	const AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
	AVCodecContext *codecContext = avcodec_alloc_context3(codec);

	if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
		fprintf(stderr, "Error copying codec parameters to context\n");
		av_free(avioContext->buffer);
		av_free(avioContext);
		avcodec_free_context(&codecContext);
		avformat_close_input(&inputFormatContext);
		return -1;
	}

	if (avcodec_open2(codecContext, codec, NULL) < 0) {
		fprintf(stderr, "Error opening codec\n");
		av_free(avioContext->buffer);
		av_free(avioContext);
		avcodec_free_context(&codecContext);
		avformat_close_input(&inputFormatContext);
		return -1;
	}

	AVFrame *frame = av_frame_alloc();
	AVFrame *rgbFrame = av_frame_alloc();

	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA , codecParameters->width, codecParameters->height, 1);
	uint8_t *buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
	av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGBA, codecParameters->width, codecParameters->height, 1);

	struct SwsContext *swsContext = sws_getContext(
		codecParameters->width, codecParameters->height, codecContext->pix_fmt,
		codecParameters->width, codecParameters->height, AV_PIX_FMT_RGBA,
		SWS_BILINEAR, NULL, NULL, NULL
	);

	int err = 0;
	AVPacket packet;

	GifskiSettings gifSettings;
	gifSettings.width = codecParameters->width;
	gifSettings.height = codecParameters->height;
	gifSettings.quality = 90;
	gifSettings.fast = 1;
	gifSettings.repeat = 0;
	gifski *gif = gifski_new(&gifSettings);
	gifski_set_write_callback(gif, cb, NULL);

	int frameNum = 0;
	while (av_read_frame(inputFormatContext, &packet) >= 0) {
		if (packet.stream_index == videoStreamIndex) {
			avcodec_send_packet(codecContext, &packet);

			if (avcodec_receive_frame(codecContext, frame) == 0) {
				sws_scale(swsContext, (const uint8_t *const *)frame->data, frame->linesize, 0,
					codecParameters->height, rgbFrame->data, rgbFrame->linesize);
				gifski_add_frame_rgba(gif, frameNum, codecParameters->width,
					codecParameters->height, *rgbFrame->data, (double)frameNum / fps);
				frameNum++;
			}
		}
		av_packet_unref(&packet);
	}

	gifski_finish(gif);
	av_free(buffer);
	sws_freeContext(swsContext);
	av_frame_free(&rgbFrame);
	av_frame_free(&frame);

	av_free(avioContext->buffer);
	av_free(avioContext);
	avcodec_free_context(&codecContext);
	avformat_close_input(&inputFormatContext);

	return 0;
}
