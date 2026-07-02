#pragma once
#include <memory>
#include <cmath>
#include <algorithm>
#include "../FlightModel/FlightModel.h"
#include "../FlightModel/MouseAimController.h"

class PlayerCamera {
public:
    void Initialize(const MyMath::Vector3& initialPos, const MyMath::Vector3& initialForward);
    void Update(float dt, FlightModel* flightModel, MouseAimController* mouseAimController, bool mouseAimEnabled);

    bool& GetFreeViewActive() { return freeViewActive_; }
    float* GetCameraDistancePtr() { return &cameraDistance_; }
    float* GetCameraHeightPtr() { return &cameraHeight_; }
    float* GetCameraPosLagPtr() { return &cameraPosLag_; }
    float* GetCameraLookLagPtr() { return &cameraLookLag_; }
    
    float GetCachedCameraYaw() const { return cachedCameraYaw_; }
    float GetCachedCameraPitch() const { return cachedCameraPitch_; }

    static MyMath::Vector3 QuaternionToEuler(const MyMath::Quaternion& q);

private:
    MyMath::Vector3 cameraCurrentPos_{};
    MyMath::Vector3 cameraLookTarget_{};

    float cameraDistance_ = 22.0f;
    float cameraHeight_ = 8.0f;
    float cameraPosLag_ = 5.0f;
    float cameraLookLag_ = 10.0f;
    float currentFov_ = 0.45f;
    bool isZoomed_ = false;

    bool freeViewActive_ = false;
    float freeViewYaw_ = 0.0f;
    float freeViewPitch_ = 0.0f;
    float freeViewYawVelocity_ = 0.0f;
    float freeViewPitchVelocity_ = 0.0f;
    float freeViewDistance_ = 25.0f;
    MyMath::Vector3 freeViewLookOffset_ = { 0.0f, 0.0f, 0.0f };

    float cachedCameraYaw_ = 0.0f;
    float cachedCameraPitch_ = 0.0f;

    MyMath::Vector3 LookAtRotation(const MyMath::Vector3& from, const MyMath::Vector3& to) const;
};
