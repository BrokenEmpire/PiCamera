#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "bcm_host.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/mmal_logging.h"
#include "PiCameraParameters.h"
#include "PiCamera.h"

PiCamera::PiCamera()
{
	vcos_log_register("Deimos.CamTest", VCOS_LOG_CATEGORY);
	bcm_host_init();
}
PiCamera::~PiCamera()
{
	if (cameraOutput && cameraOutput->is_enabled)
		mmal_port_parameter_set_boolean(cameraOutput, MMAL_PARAMETER_CAPTURE, 0);

	if (connection && connection->is_enabled)
		mmal_connection_disable(connection);

	if (connection)
		mmal_connection_destroy(connection);

	if (userdata && userdata->file_handle)
	{
		fflush(userdata->file_handle);
		fclose(userdata->file_handle);
	}

	if (userdata && userdata->imv_file_handle)
	{
		fflush(userdata->imv_file_handle);
		fclose(userdata->imv_file_handle);
	}

	if (encoderOutput && encoderOutput->is_enabled)
		mmal_port_disable(encoderOutput);

	if (encoderInput && encoderInput->is_enabled)
		mmal_port_disable(encoderInput);

	if (encoder && encoder->is_enabled)
		mmal_component_disable(encoder);

	if (encoderPool)
		mmal_port_pool_destroy(encoderOutput, encoderPool);

	if (encoder)
		mmal_component_destroy(encoder);

	if (cameraOutput && cameraOutput->is_enabled)
		mmal_port_disable(cameraOutput);

	if (camera && camera->is_enabled)
		mmal_component_disable(camera);

	if (camera)
		mmal_component_destroy(camera);

	if (userdata)
	{
		userdata->file_handle = NULL;
		userdata->imv_file_handle = NULL;
		userdata->pool = NULL;

		delete userdata;
	}

	bcm_host_deinit();
	vcos_log_unregister(VCOS_LOG_CATEGORY);

	connection = NULL;
	encoderPool = NULL;
	encoderInput = NULL;
	encoderOutput = NULL;
	cameraOutput = NULL;
	encoder = NULL;
	camera = NULL;
	userdata = NULL;
}

void PiCamera::Initialize()
{
	initializeCamera();
	initializeEncoder();
	initializeConnection();

	userdata = new MMAL_PORT_USERDATA();
	userdata->pool = encoderPool;
	userdata->file_handle = fopen("testvideo.h264", "wb");
	userdata->imv_file_handle = fopen("imuData", "wb");

	encoderOutput->userdata = userdata;
	mmal_port_enable(encoderOutput, encoderCallback);

	while (mmal_queue_length(encoderPool->queue))
	{
		MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(encoderPool->queue);
		mmal_port_send_buffer(encoderOutput, buffer);
	}

	mmal_port_parameter_set_boolean(cameraOutput, MMAL_PARAMETER_CAPTURE, 1);
}
void PiCamera::initializeCamera()
{
	mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
	cameraOutput = camera->output[MMAL_CAMERA_VIDEO_PORT];

	mmal_port_parameter_set(camera->control, &CameraNumberParameter.hdr);
	mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, PARAMETER_SENSOR_MODE);
	mmal_port_enable(camera->control, cameraCallback);
	mmal_port_parameter_set(camera->control, &ConfigurationParameter.hdr);

	cameraOutput->buffer_num = PARAMETER_BUFFER_COUNT;
	cameraOutput->format->encoding_variant = MMAL_ENCODING_I420;
	cameraOutput->format->encoding = MMAL_ENCODING_OPAQUE;
	cameraOutput->format->es->video.width = VCOS_ALIGN_UP(PARAMETER_WIDTH, 32);
	cameraOutput->format->es->video.height = VCOS_ALIGN_UP(PARAMETER_HEIGHT, 16);
	cameraOutput->format->es->video.crop.x = 0;
	cameraOutput->format->es->video.crop.y = 0;
	cameraOutput->format->es->video.crop.width = PARAMETER_WIDTH;
	cameraOutput->format->es->video.crop.height = PARAMETER_HEIGHT;
	cameraOutput->format->es->video.frame_rate.num = PARAMETER_FRAMERATE_VIDEO;
	cameraOutput->format->es->video.frame_rate.den = PARAMETER_FRAMERATE_DENOMINATOR;

	mmal_port_format_commit(cameraOutput);
	mmal_component_enable(camera);
}
void PiCamera::initializeEncoder()
{
	mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder);

	encoderInput = encoder->input[MMAL_ENCODER_INPUT_PORT];
	encoderOutput = encoder->output[MMAL_ENCODER_OUTPUT_PORT];

	mmal_format_copy(encoderOutput->format, encoderInput->format);

	encoderOutput->format->encoding = MMAL_ENCODING_H264;
	encoderOutput->format->bitrate = PARAMETER_ENCODER_BITRATE;
	encoderOutput->buffer_size = encoderOutput->buffer_size_recommended;
	encoderOutput->format->es->video.frame_rate.num = 0;
	encoderOutput->format->es->video.frame_rate.den = 1;

	mmal_port_format_commit(encoderOutput);

	videoProfileParameter.profile[0].profile = MMAL_VIDEO_PROFILE_H264_HIGH;
	videoProfileParameter.profile[0].level = MMAL_VIDEO_LEVEL_H264_4;

	mmal_port_parameter_set(encoderOutput, &rateControlParameter.hdr);
	mmal_port_parameter_set(encoderOutput, &videoProfileParameter.hdr);

	mmal_port_parameter_set_boolean(encoderInput, MMAL_PARAMETER_VIDEO_IMMUTABLE_INPUT, 1);
	mmal_port_parameter_set_boolean(encoderOutput, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_HEADER, 0);
	mmal_port_parameter_set_boolean(encoderOutput, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS, 1);

	mmal_component_enable(encoder);
	encoderPool = mmal_port_pool_create(encoderOutput, encoderOutput->buffer_num, encoderOutput->buffer_size);
}
void PiCamera::initializeConnection()
{
	mmal_connection_create(&connection, cameraOutput, encoderInput, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
	mmal_connection_enable(connection);
}

void PiCamera::cameraCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	mmal_buffer_header_release(buffer);
}
void PiCamera::encoderCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	if (buffer->length)
	{
		mmal_buffer_header_mem_lock(buffer);

		if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO)
			fwrite(buffer->data, 1, buffer->length, port->userdata->imv_file_handle);
		else
			fwrite(buffer->data, 1, buffer->length, port->userdata->file_handle);

		mmal_buffer_header_mem_unlock(buffer);
	}

	mmal_buffer_header_release(buffer);

	if (port->is_enabled)
	{
		MMAL_BUFFER_HEADER_T *_buffer = mmal_queue_get(port->userdata->pool->queue);
		mmal_port_send_buffer(port, _buffer);
	}
}