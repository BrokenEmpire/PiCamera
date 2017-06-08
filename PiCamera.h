#ifndef _PICAMERA_H_
#define _PICAMERA_H_

struct MMAL_POOL_T;
struct MMAL_PORT_T;
struct MMAL_BUFFER_HEADER_T;
struct MMAL_CONNECTION_T;
struct MMAL_COMPONENT_T;
typedef struct MMAL_PORT_USERDATA_T
{
	FILE *file_handle;
	FILE *imv_file_handle;
	MMAL_POOL_T *pool;
} MMAL_PORT_USERDATA;
class PiCamera
{
public:
	PiCamera();
	~PiCamera();

	void Initialize();

private:
	MMAL_COMPONENT_T *camera;
	MMAL_COMPONENT_T *encoder;
	MMAL_CONNECTION_T *connection;
	MMAL_PORT_USERDATA *userdata;
	MMAL_PORT_T *cameraOutput;
	MMAL_PORT_T *encoderInput;
	MMAL_PORT_T *encoderOutput;
	MMAL_POOL_T *encoderPool;

	void initializeCamera();
	void initializeEncoder();
	void initializeConnection();

	static void cameraCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	static void encoderCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
};
#endif