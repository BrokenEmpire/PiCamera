#ifndef _PICAMERA_PARAMETERS_H_
#define _PICAMERA_PARAMETERS_H_

#define MMAL_CAMERA_PREVIEW_PORT		0
#define MMAL_CAMERA_VIDEO_PORT			1
#define MMAL_CAMERA_CAPTURE_PORT		2
#define MMAL_ENCODER_INPUT_PORT			0
#define MMAL_ENCODER_OUTPUT_PORT		0

#define PARAMETER_WIDTH					1920
#define PARAMETER_HEIGHT				1080
#define	PARAMETER_CAMERA_ID				0
#define PARAMETER_SENSOR_MODE			0
#define PARAMETER_FRAMERATE_VIDEO		30
#define PARAMETER_FRAMERATE_DENOMINATOR	1
#define PARAMETER_BUFFER_COUNT			3

#define PARAMETER_ENCODER_BITRATE		17000000
#define PARAMETER_ENCODER_H264_LEVEL	MMAL_VIDEO_LEVEL_H264_4

MMAL_PARAMETER_INT32_T CameraNumberParameter =
{
	{ MMAL_PARAMETER_CAMERA_NUM, sizeof(CameraNumberParameter) },
	PARAMETER_CAMERA_ID
};

MMAL_PARAMETER_CAMERA_CONFIG_T ConfigurationParameter =
{
	{ MMAL_PARAMETER_CAMERA_CONFIG, sizeof(ConfigurationParameter) },
	.max_stills_w = PARAMETER_WIDTH,
	.max_stills_h = PARAMETER_HEIGHT,
	.stills_yuv422 = 0,
	.one_shot_stills = 0,
	.max_preview_video_w = PARAMETER_WIDTH,
	.max_preview_video_h = PARAMETER_HEIGHT,
	.num_preview_video_frames = 3,
	.stills_capture_circular_buffer_height = 0,
	.fast_preview_resume = 0,
	.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RAW_STC
};

MMAL_PARAMETER_VIDEO_RATECONTROL_T rateControlParameter =
{
	{ MMAL_PARAMETER_RATECONTROL, sizeof(rateControlParameter) },
	MMAL_VIDEO_RATECONTROL_DEFAULT
};

MMAL_PARAMETER_VIDEO_PROFILE_T videoProfileParameter =
{
	{ MMAL_PARAMETER_PROFILE, sizeof(videoProfileParameter) }
};

#endif