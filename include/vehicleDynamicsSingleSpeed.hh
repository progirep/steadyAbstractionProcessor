#ifndef __VEHICLE_DYNAMICS_SINGLE_SPEED_HH__
#define __VEHICLE_DYNAMICS_SINGLE_SPEED_HH__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <array>
#include <tuple>
#include <vector>
#include <cmath>
#include <functional>
#include "dynamics.hh"
#include "tools.hh"
#include "scots.hh"
#include "RungeKutta4.hh"
#undef fail

class VehicleDynamicsSingleSpeed : public Dynamics {
private:
public:

    static constexpr unsigned int getStateNofDimensions() {return 3;}
    static constexpr unsigned int getInputNofDimensions() {return 1;}
    static constexpr std::array<bool,3> getDimensionIsTranslationInvariant() {return {true, true, false};}
    static constexpr std::array<bool,3> getDimensionIsWrapAround() {return {false, false, true};}
    static constexpr std::array<double,3> getDimensionWrapAroundLimits() {return {0.0, 0.0, 2.0*M_PI};}

    typedef std::array<double,3> continuous_state_type;
    typedef std::array<double,1> continuous_input_type;
    typedef std::array<int,3> discrete_state_type;
    typedef std::array<int,1> discrete_input_type;

    typedef IntArrayHasher<3> discrete_state_type_hasher;
    typedef IntArrayHasher<1> discrete_input_type_hasher;

    static std::array<std::string,3> getStateVariableNames() {return {"xbit","ybit","curHeading"};}
    static std::array<std::string,1> getInputVariableNames() {return {"rot"};}


    static std::function<void(continuous_state_type &x, continuous_input_type &u)> getSystemDynamicsPost(double tau) {
        return [tau](continuous_state_type &x, continuous_input_type &u)  {

            /* the ode describing the vehicle */
            auto rhs =[](continuous_state_type& xx,  const continuous_state_type &x, continuous_input_type &u) {

                xx[0] = std::sin(x[2]);
                xx[1] = std::cos(x[2]);
                xx[2] = u[0];

            };
            scots::runge_kutta_fixed4(rhs,x,u,getStateNofDimensions(),tau,10);
        };
    };

    static std::function<void(continuous_state_type &r, const continuous_state_type &, const continuous_input_type &)> getGrowthBoundFunction(double tau) {
        return [tau](continuous_state_type &r, const continuous_state_type &, const continuous_input_type &) {
            r[0] = r[0] + (3*r[2])*tau;
            r[1] = r[1] + (3*r[2])*tau;
            r[2] = r[2];
        };
    }

};


#endif
