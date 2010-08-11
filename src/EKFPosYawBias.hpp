#ifndef __EKF_POSITIONYAWBIAS_HPP__
#define __EKF_POSITIONYAWBIAS_HPP__

#include <Eigen/Core>
#include <Eigen/Geometry>
#include "ExtendedKalmanFilter.hpp"
#include <fault_detection/FaultDetection.hpp>


namespace pose_estimator {
    class State
    {
    public:
	static const int SIZE = 4;
	typedef Eigen::Matrix<double,SIZE,1> vector_t;

    protected:
	vector_t _x; 
	Eigen::Block<vector_t, 3, 1> _xi;
	Eigen::Block<vector_t, 1, 1> _yaw;

    public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	State() :
	    _x(vector_t::Zero()), _xi( _x.segment<3>(0) ),_yaw(_x.segment<1>(3)) {};

	vector_t& vector() {return _x;};
	Eigen::Block<vector_t, 3, 1>& xi() {return _xi;}	
	Eigen::Block<vector_t, 1, 1>& yaw() {return _yaw;}
    };

    class EKFPosYawBias
    {
    public:
      	/** ensure alignment for eigen vector types */
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	

	static const int THRESHOLD_2D_99 = 99;
	static const int THRESHOLD_2D_95 = 95;
	static const int THRESHOLD_2D_0 = 0;
	
	static const int INPUT_SIZE = 3;
	static const int MEASUREMENT_SIZE_SCAN_MATCH = 4;
	static const int MEASUREMENT_SIZE_GPS = 3;

	/** state estimate */
	State x;
	
	/** covariance matrix */
	Eigen::Matrix<double, State::SIZE, State::SIZE> P;
	
	bool reject_GPS_observation;
	bool reject_scan_match_observation; 
	
    private:
	
	int reject_GPS_threshold; 
	int reject_icp_threshold; 
	
      	/**fault detection libary */ 
	fault_detection::ChiSquared* chi_square; 
	
	/** Instance of the Extended Kalman filter*/
	ExtendedKalmanFilter::EKF<State::SIZE,INPUT_SIZE,MEASUREMENT_SIZE_SCAN_MATCH>* filter_scan_match;
	
	/** Instance of the Extended Kalman filter*/
	ExtendedKalmanFilter::EKF<State::SIZE,INPUT_SIZE,MEASUREMENT_SIZE_GPS>* filter_gps;
	
	/** process noise */
	Eigen::Matrix<double, State::SIZE, State::SIZE> Q;
	
	
    public:

	EKFPosYawBias();
	~EKFPosYawBias();
  
	/** sets the rejection threshold for the GPS sample */ 
	void setGpsRejectThreshold(int threshold);
	
	/** sets the rejection threshold for the GPS sample */ 
	void setIcpRejectThreshold(int threshold);
	
	/** update step taking velocity in world frame without the bias correction */
	void predict(const Eigen::Vector3d &translation_world );

	/** correction step, taking absolute position data */
	void correctionGps(const Eigen::Matrix<double, MEASUREMENT_SIZE_GPS, 1> &p, Eigen::Transform3d C_w2gw_without_bias, Eigen::Matrix<double, MEASUREMENT_SIZE_GPS, MEASUREMENT_SIZE_GPS> R_GPS  );

	/** correction step, taking absolute position data */
	void correctionScanMatch(const Eigen::Matrix<double, MEASUREMENT_SIZE_SCAN_MATCH, 1> &p, Eigen::Matrix<double, MEASUREMENT_SIZE_SCAN_MATCH, MEASUREMENT_SIZE_SCAN_MATCH> R_scan_match  );

	/** set the process noise the frame should be world corrected by bias [velocity,  bias] */ 
	void processNoise(const Eigen::Matrix<double, State::SIZE, State::SIZE> &Q); 
	
	/** set the inital values for state x and covariance P */
	void init(const Eigen::Matrix<double, State::SIZE, State::SIZE> &P, const Eigen::Matrix<double,State::SIZE,1> &x); 
	
	
      private: 
	
	/**jacobian state transition*/ 
	Eigen::Matrix<double, State::SIZE, State::SIZE> jacobianF( const Eigen::Vector3d &translation_world );
	
	/**jacobian observation model*/ 
	Eigen::Matrix<double, MEASUREMENT_SIZE_SCAN_MATCH, State::SIZE>  jacobianScanMatchObservation();

	/**jacobian of the GPS observation */ 
	Eigen::Matrix<double, MEASUREMENT_SIZE_GPS, State::SIZE>  jacobianGpsObservation(Eigen::Transform3d C_w2gw_without_bias);	
	
	
    };
}

#endif
