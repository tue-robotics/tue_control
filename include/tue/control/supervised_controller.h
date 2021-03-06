#ifndef TUE_CONTROL_SUPERVISED_CONTROLLER_H_
#define TUE_CONTROL_SUPERVISED_CONTROLLER_H_

#include <memory>

#include <tue/config/configuration.h>
#include <tue/control/controller_input.h>

namespace tue
{
namespace control
{

class Controller;
struct ControllerInput;
struct ControllerOutput;

// ----------------------------------------------------------------------------------------------------

enum ControllerStatus
{
    UNINITIALIZED = 0,
    HOMING = 1,
    ACTIVE = 2,
    IDLE = 3,
    ERROR = 4
};

// ----------------------------------------------------------------------------------------------------

enum ControllerEvent
{
    NONE = 0,
    START_HOMING = 2,
    STOP_HOMING = 3,
    SET_ERROR = 4,
    ENABLE = 5,
    DISABLE = 6
};

// ----------------------------------------------------------------------------------------------------

// Wraps Controller with safety and homing functionality

class SupervisedController
{

public:

    SupervisedController();

    ~SupervisedController();

    SupervisedController(const SupervisedController&) = delete;

    SupervisedController& operator=(const SupervisedController&) = delete;


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Configuration

    void setController(const std::shared_ptr<Controller>& controller) { controller_ = controller; }

    void configure(tue::Configuration& config, double dt);


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Update

    void update(double measurement);

    void updateHoming(double measurement, ControllerOutput& output);


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Change state / status

    void setReference(double pos, double vel = 0, double acc = 0)
    {
        input_.pos_reference = pos;
        input_.vel_reference = vel;
        input_.acc_reference = acc;
    }

    void startHoming() { event_ = START_HOMING; }

    void stopHoming(double current_pos) { event_ = STOP_HOMING; homed_measurement_ = current_pos; }

    void setError(const std::string& error_msg) { event_ = SET_ERROR; error_msg_ = error_msg; }

    void disable() { event_ = DISABLE; }

    void enable() { event_ = ENABLE; }


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Getters

    ControllerStatus status() const { return status_; }

    const char* status_string() const
    {
        static const char* STATUS_STRING[] = { "UNINITIALIZED", "HOMING", "ACTIVE", "INACTIVE", "ERROR" };
        return STATUS_STRING[status()];
    }

    double error() const { return error_; }

    double measurement() const { return input_.measurement; }

    double output() const { return output_; }

    bool accepts_references() const { return status() == ACTIVE; }

    const std::string& name() const;

    const std::string& error_message() const { return error_msg_; }

    double reference_position() const { return input_.pos_reference; }

    double reference_velocity() const { return input_.vel_reference; }

    double reference_acceleration() const { return input_.acc_reference; }

    bool is_homed() const { return homed_; }

    bool is_homable() const { return homable_; }

private:

    double dt_;

    ControllerStatus status_;

    ControllerEvent event_;

    std::shared_ptr<Controller> controller_;

    std::string error_msg_;

    ControllerInput input_;

    bool homed_;

    bool homable_;

    double error_;

    double output_;

    void checkTransitions(double raw_measurements);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Safety

    double output_saturation_;
    double max_error_;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Homing

    double measurement_offset;

    double homing_max_vel_;
    double homing_max_acc_;

    double homing_pos;
    double homing_vel;
    double homed_measurement_;

};

} // end namespace tue

} // end namespace control

#endif
