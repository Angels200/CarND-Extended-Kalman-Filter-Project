#include "FusionEKF.h"
//#include "tools.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/*
 * Class Constructor.
 */
FusionEKF::FusionEKF() {
  is_initialized_ = false;

  previous_timestamp_ = 0;

  // initializing matrices
  R_laser_ = MatrixXd(2, 2);
  R_radar_ = MatrixXd(3, 3);
  H_laser_ = MatrixXd(2, 4);
  Hj_ = MatrixXd(3, 4);

  //measurement covariance matrix - laser
  R_laser_ << 0.0225, 0,
        0, 0.0225;

  //measurement covariance matrix - radar
  R_radar_ << 0.09, 0, 0,
        0, 0.0009, 0,
        0, 0, 0.09;

  /**
  TODO:
    * Finish initializing the FusionEKF.
    * Set the process and measurement noises
  */
  H_laser_ = MatrixXd(2, 4);
      H_laser_ << 1, 0, 0, 0,
    		  	  0, 1, 0, 0;

  //state covariance matrix P
  MatrixXd P_ = MatrixXd(4, 4);
  P_ << 1, 0, 0, 0,
		  0, 1, 0, 0,
		  0, 0, 1000, 0,
		  0, 0, 0, 1000;

  MatrixXd F_ = MatrixXd(4, 4);
  MatrixXd Q_ = MatrixXd(4, 4);

  VectorXd x_ = VectorXd(4);
  x_ << 1, 1, 1, 1;

  ekf_.Init(x_, P_, F_, H_laser_, R_laser_, Q_);

  process_noise_ax_ = 9;
  process_noise_ay_ = 9;

}

/**
* Destructor.
*/
FusionEKF::~FusionEKF() {}

void FusionEKF::ProcessMeasurement(const MeasurementPackage &measurement_pack) {


  /*****************************************************************************
   *  Initialization
   ****************************************************************************/
  if (!is_initialized_) {
    /**
    TODO:
      * Initialize the state ekf_.x_ with the first measurement.
      * Create the covariance matrix.
      * Remember: you'll need to convert radar from polar to cartesian coordinates.
    */
    // first measurement
    cout << "EKF: " << endl;
    ekf_.x_ = VectorXd(4);
    ekf_.x_ << 1, 1, 1, 1;

    double px;
    double py;

    if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
      /**
      Convert radar from polar to cartesian coordinates and initialize state.
      */
    	float ro     = measurement_pack.raw_measurements_(0);
	    float phi    = measurement_pack.raw_measurements_(1);
	    float ro_dot = measurement_pack.raw_measurements_(2);
	    ekf_.x_(0) = ro     * cos(phi);
	    ekf_.x_(1) = ro     * sin(phi);
	    ekf_.x_(2) = ro_dot * cos(phi);
    	ekf_.x_(3) = ro_dot * sin(phi);
    }
    else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
      /**
      Initialize state.
      */
    	px = measurement_pack.raw_measurements_[0];
    	py = measurement_pack.raw_measurements_[1];
    }

    ekf_.x_ << px, py, 0, 0;
    previous_timestamp_ = measurement_pack.timestamp_;
    // done initializing, no need to predict or update
    is_initialized_ = true;
    return;
  }

  /*****************************************************************************
   *  Prediction
   ****************************************************************************/

  /**
   TODO:
     * Update the state transition matrix F according to the new elapsed time.
      - Time is measured in seconds.
     * Update the process noise covariance matrix.
     * Use noise_ax = 9 and noise_ay = 9 for your Q matrix.
   */
  double dt = (measurement_pack.timestamp_ - previous_timestamp_) / 1000000.0;
  previous_timestamp_ = measurement_pack.timestamp_;

  ekf_.F_ << 1, 0, dt, 0,
		  0, 1, 0, dt,
		  0, 0, 1, 0,
		  0, 0, 0, 1;

  ekf_.Q_ <<  pow(dt, 4.0d) / 4 * process_noise_ax_, 0, pow(dt, 3.0d) / 2 * process_noise_ax_, 0,
			  0, pow(dt, 4.0d) / 4 * process_noise_ay_, 0, pow(dt, 3.0d) / 2 * process_noise_ay_,
			  pow(dt, 3.0d) / 2 * process_noise_ax_, 0, dt * dt * process_noise_ax_, 0,
			  0, pow(dt, 3.0d) / 2 * process_noise_ay_, 0, dt * dt * process_noise_ay_;

  ekf_.Predict();



  /*****************************************************************************
   *  Update
   ****************************************************************************/

  /**
   TODO:
     * Use the sensor type to perform the update step.
     * Update the state and covariance matrices.
   */

  if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
          // Radar updates
          ekf_.R_ = R_radar_;
          ekf_.H_ = tools.CalculateJacobian(ekf_.x_);
          ekf_.UpdateEKF(measurement_pack.raw_measurements_);
      } else {
          // Laser updates
          ekf_.R_ = R_laser_;
          ekf_.H_ = H_laser_;
          ekf_.Update(measurement_pack.raw_measurements_);
  }

  // print the output
  cout << "x_ = " << ekf_.x_ << endl;
  cout << "P_ = " << ekf_.P_ << endl;
}
