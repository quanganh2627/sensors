/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_LIGHT_SENSOR_H
#define ANDROID_LIGHT_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

#define APDS9900_LUX_OUTPUT_SCALE 10.0
#ifdef TARGET_MFLD_GI
#define APDS9900_LIGHT_TRANS 0.3
#else
#define APDS9900_LIGHT_TRANS 1
#endif

/*****************************************************************************/

struct input_event;

class LightSensor : public SensorBase {
    int mEnabled;
    InputEventCircularReader mInputReader;
    sensors_event_t mPendingEvent;
    bool mHasPendingEvent;

    int setInitialState();
    float mPreviousLight;

public:
    LightSensor();
    virtual ~LightSensor();
    virtual int readEvents(sensors_event_t* data, int count);
    virtual bool hasPendingEvents() const;
    virtual int enable(int32_t handle, int enabled);
};

/*****************************************************************************/

#endif  // ANDROID_LIGHT_SENSOR_H
