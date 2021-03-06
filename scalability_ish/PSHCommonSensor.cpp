#include "PSHCommonSensor.hpp"

int PSHCommonSensor::getPollfd()
{
 		log_message(CRITICAL,"getPollfd()\n");
       if (pollfd >= 0)
                return pollfd;

        if (methods.psh_open_session == NULL || methods.psh_get_fd == NULL) {
                log_message(CRITICAL,"psh_open_session/psh_get_fd not initialized!");
                return -1;
        }
		log_message(CRITICAL,"before getType()\n");

        psh_sensor_t PSHType = SensorHubHelper::getType(device.getType(), device.getSubname());
        
		log_message(CRITICAL,"before psh_open_session() %d\n", PSHType);
		sensorHandle = methods.psh_open_session(PSHType);
        if (sensorHandle == NULL) {
                log_message(CRITICAL,"psh_open_session error!");
                return -1;
        }
		log_message(CRITICAL,"before psh_get_fd() \n");

        pollfd = methods.psh_get_fd(sensorHandle);

        return pollfd;
}

int PSHCommonSensor::activate(int handle, int enabled) {
        if (methods.psh_start_streaming == NULL || methods.psh_stop_streaming == NULL || sensorHandle == NULL) {
                log_message(CRITICAL,"psh_start_streaming/psh_stop_streaming/sensorHandle not initialized!");
                return -1;
        }

        if(activated && enabled == 0) {
                error_t err = methods.psh_stop_streaming(sensorHandle);
                if (err != ERROR_NONE) {
                        log_message(CRITICAL,"psh_stop_streaming error %d", err);
                        return -1;
                }
                activated = false;
        } else if (enabled)
                activated = true;

        return 0;
}

int PSHCommonSensor::setDelay(int handle, int64_t ns) {
        int dataRate = 5;
        int bufferDelay = 0;
        streaming_flag flag = STOP_WHEN_SCREEN_OFF;
        int delay = 200;
        int minDelay = device.getMinDelay() / US_TO_MS;

        if (methods.psh_start_streaming == NULL || methods.psh_stop_streaming == NULL) {
                log_message(CRITICAL,"psh_start_streaming/psh_stop_streaming not initialized!");
                return -1;
        }

        if (!activated)
                return 0;

        if (ns / 1000 != SENSOR_NOPOLL)
                delay = ns / NS_TO_MS;
        else
                delay = 200;

        if (delay < minDelay)
                delay = minDelay;

        if (delay != 0)
                dataRate = 1000 / delay;

        if (dataRate == 0 || minDelay == 0)
                dataRate = 1;

        /* Some PSH session need to set different rate and delay */
        SensorHubHelper::getStartStreamingParameters(device.getType(), dataRate, bufferDelay, flag);

        if (dataRate < 0) {
                log_message(CRITICAL,"Invalid delay: %d", delay);
                return -1;
        }

        /* Some PSH session need to set property */
        if (!SensorHubHelper::setPSHPropertyIfNeeded(device.getType(), methods, sensorHandle)) {
                log_message(CRITICAL,"Set property failed for sensor type %d", device.getType());
                return -1;
        }

        error_t err;
        /* Wait for libsensorhub interface sync */
        if (flag == STOP_WHEN_SCREEN_OFF)
                err = methods.psh_start_streaming(sensorHandle, dataRate, bufferDelay);
        else
                err = methods.psh_start_streaming_with_flag(sensorHandle, dataRate, bufferDelay, flag);
        if (err != ERROR_NONE) {
                log_message(CRITICAL,"psh_start_streaming(_with_flag) error %d name:%s handle: %x %d %d",
                     err, device.getName(), sensorHandle, dataRate, flag);
                return -1;
        }

        last_timestamp = getTimestamp();
        return 0;
}

int PSHCommonSensor::getData(std::queue<sensors_event_t> &eventQue) {
        int count = 32;
		log_message(WARNING, "got Data from sensor %d\n", device.getType());
        count = SensorHubHelper::readSensorhubEvents(pollfd, sensorhubEvent, count, device.getType(), last_timestamp);
		log_message(WARNING, "count was %d\n", count);
        for (int i = 0; i < count; i++) {
                if (device.getType() == SENSOR_TYPE_STEP_COUNTER) {
                        event.u64.step_counter = sensorhubEvent[i].step_counter;
                } else {
                event.data[device.getMapper(AXIS_X)] = SensorHubHelper::ConvertToFloat(sensorhubEvent[i].data[0], device.getType());//sensorhubEvent[i].data[0] * device.getScale(AXIS_X);
                event.data[device.getMapper(AXIS_Y)] = SensorHubHelper::ConvertToFloat(sensorhubEvent[i].data[1], device.getType());//sensorhubEvent[i].data[1] * device.getScale(AXIS_Y);
                event.data[device.getMapper(AXIS_Z)] = SensorHubHelper::ConvertToFloat(sensorhubEvent[i].data[2], device.getType());//sensorhubEvent[i].data[2] * device.getScale(AXIS_Z);
                event.data[device.getMapper(AXIS_W)] = SensorHubHelper::ConvertToFloat(sensorhubEvent[i].data[3], device.getType());//sensorhubEvent[i].data[3] * device.getScale(AXIS_W);
                if (sensorhubEvent[i].accuracy != 0)
                        event.acceleration.status = sensorhubEvent[i].accuracy;
                }
                event.timestamp = sensorhubEvent[i].timestamp;
				//log_message(WARNING, "Orig data was [%02X, %02X, %02X] \n",  sensorhubEvent[i].data[0], sensorhubEvent[i].data[1],sensorhubEvent[i].data[2]);
            	log_message(WARNING, "Converted data was [%f, %f, %f] \n",  event.data[0], event.data[1],event.data[2]);
				eventQue.push(event);
        }

        return 0;
}

bool PSHCommonSensor::selftest() {
			log_message(CRITICAL,"selftest()\n");

        if (getPollfd() >= 0)
                return true;
        return false;
}
