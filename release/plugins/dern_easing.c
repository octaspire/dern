/******************************************************************************
Octaspire Dern - Programming language
Copyright 2017 www.octaspire.com

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
******************************************************************************/
#include "octaspire-dern-amalgamated.c"

/////////////////////////////// easing //////////////////////////////////////////
/******************************************************************************
This work is based on Robert Penner's 'penner_easing_as1.txt'. The copyright
and license of the original work is:

Open source under the BSD License.

Copyright © 2001 Robert Penner
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  • Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  • Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  • Neither the name of the author nor the names of contributors may be used to
    endorse or promote products derived from this software without specific prior
    written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This is not the original version, but a modified one, converted from
'penner_easing_as1.txt' into C99 version by www.octaspire.com.
These modifications are released also under the BSD License:

Copyright © 2017 www.octaspire.com
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  • Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  • Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  • Neither the name of the author nor the names of contributors may be used to
    endorse or promote products derived from this software without specific prior
    written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/
#include <math.h>

#ifndef M_PI
#define M_PI (3.14159265359)
#endif

double const OCTASPIRE_EASING_DEFAULT_10_PERCENT_OVERSHOOT = 1.70158;
double const OCTASPIRE_EASING_DEFAULT_AMPLITUDE            = 0;
double const OCTASPIRE_EASING_DEFAULT_PERIOD               = 0;

double octaspire_easing_linear(double const t, double const b, double const c, double const d);
double octaspire_easing_in_quad(double t, double const b, double const c, double const d);
double octaspire_easing_out_quad(double t, double const b, double const c, double const d);
double octaspire_easing_in_out_quad(double t, double const b, double const c, double const d);
double octaspire_easing_in_cubic(double t, double const b, double const c, double const d);
double octaspire_easing_out_cubic(double t, double const b, double const c, double const d);
double octaspire_easing_in_out_cubic(double t, double const b, double const c, double const d);
double octaspire_easing_in_quart(double t, double const b, double const c, double const d);
double octaspire_easing_out_quart(double t, double const b, double const c, double const d);
double octaspire_easing_in_out_quart(double t, double const b, double const c, double const d);
double octaspire_easing_in_quint (double t, double const b, double const c, double const d);
double octaspire_easing_out_quint(double t, double const b, double const c, double const d);
double octaspire_easing_in_out_quint(double t, double const b, double const c, double const d);
double octaspire_easing_in_sine(double const t, double const b, double const c, double const d);
double octaspire_easing_out_sine(double const t, double const b, double const c, double const d);
double octaspire_easing_in_out_sine(double const t, double const b, double const c, double const d);
double octaspire_easing_in_expo(double const t, double const b, double const c, double const d);
double octaspire_easing_out_expo(double const t, double const b, double const c, double const d);
double octaspire_easing_in_out_expo(double t, double const b, double const c, double const d);
double octaspire_easing_in_circ(double t, double const b, double const c, double const d);
double octaspire_easing_out_circ(double t, double const b, double const c, double const d);
double octaspire_easing_in_out_circ(double t, double const b, double const c, double const d);

double octaspire_easing_in_elastic(
    double t,
    double const b,
    double const c,
    double const d,
    double a,
    double p);

double octaspire_easing_out_elastic(
    double t,
    double const b,
    double const c,
    double const d,
    double a,
    double p);

double octaspire_easing_in_out_elastic(
    double t,
    double const b,
    double const c,
    double const d,
    double a,
    double p);

double octaspire_easing_in_back(
    double t,
    double const b,
    double const c,
    double const d,
    double const s);

double octaspire_easing_out_back(
    double t,
    double const b,
    double const c,
    double const d,
    double const s);

double octaspire_easing_in_out_back(
    double t,
    double const b,
    double const c,
    double const d,
    double s);

double octaspire_easing_in_bounce(double const t, double const b, double const c, double const d);
double octaspire_easing_out_bounce(double t, double const b, double const c, double const d);

double octaspire_easing_in_out_bounce(
    double const t,
    double const b,
    double const c,
    double const d);



// simple linear tweening - no easing
// t: current time, b: beginning value, c: change in value, d: duration
double octaspire_easing_linear(double const t, double const b, double const c, double const d)
{
    return c*t/d + b;
}


///////////// QUADRATIC EASING: t^2 ///////////////////
// quadratic easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in value, d: duration
// t and d can be in frames or seconds/milliseconds
double octaspire_easing_in_quad(double t, double const b, double const c, double const d)
{
    t /= d;
    return c*t*t + b;
}

// quadratic easing out - decelerating to zero velocity
double octaspire_easing_out_quad(double t, double const b, double const c, double const d)
{
    t /= d;
    return -c*t*(t-2) + b;
}

// quadratic easing in/out - acceleration until halfway, then deceleration
double octaspire_easing_in_out_quad(double t, double const b, double const c, double const d)
{
    t /= (d * 0.5);

    if (t < 1)
    {
        return (c / 2) * pow(t, 2) + b;
    }

    return -c / 2 * ((t-1) * (t-3) - 1) + b;
}


///////////// CUBIC EASING: t^3 ///////////////////////
// cubic easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in value, d: duration
// t and d can be frames or seconds/milliseconds
double octaspire_easing_in_cubic(double t, double const b, double const c, double const d)
{
    t /= d;
    return c*t*t*t + b;
}

// cubic easing out - decelerating to zero velocity
double octaspire_easing_out_cubic(double t, double const b, double const c, double const d)
{
    t = (t / d) - 1;
    return c * (pow(t, 3) + 1) + b;
}

// cubic easing in/out - acceleration until halfway, then deceleration
double octaspire_easing_in_out_cubic(double t, double const b, double const c, double const d)
{
    t /= (d * 0.5);

    if (t < 1)
    {
        return c/2*t*t*t + b;
    }

    t -= 2;

    return c/2*((t*t*t) + 2) + b;
}


///////////// QUARTIC EASING: t^4 /////////////////////
// quartic easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in value, d: duration
// t and d can be frames or seconds/milliseconds
double octaspire_easing_in_quart(double t, double const b, double const c, double const d)
{
    t /= d;
    return c*(t*t*t*t) + b;
}

// quartic easing out - decelerating to zero velocity
double octaspire_easing_out_quart(double t, double const b, double const c, double const d)
{
    t = (t / d) - 1;
    return -c * (pow(t, 4) - 1) + b;
}

// quartic easing in/out - acceleration until halfway, then deceleration
double octaspire_easing_in_out_quart(double t, double const b, double const c, double const d)
{
    t /= (d * 0.5);
    if (t < 1) return c/2*t*t*t*t + b;
    t -= 2;
    return -c/2 * ((t*t*t*t) - 2) + b;
}


///////////// QUINTIC EASING: t^5  ////////////////////
// quintic easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in value, d: duration
// t and d can be frames or seconds/milliseconds
double octaspire_easing_in_quint (double t, double const b, double const c, double const d)
{
    t /= d;
    return c * (t*t*t*t*t) + b;
}

// quintic easing out - decelerating to zero velocity
double octaspire_easing_out_quint(double t, double const b, double const c, double const d)
{
    t = (t / d) -1;
    return c*((t*t*t*t*t) + 1) + b;
}

// quintic easing in/out - acceleration until halfway, then deceleration
double octaspire_easing_in_out_quint(double t, double const b, double const c, double const d)
{
    t /= (d * 0.5);
    if (t < 1) return c/2*t*t*t*t*t + b;
    t -= 2;
    return c/2*((t*t*t*t*t) + 2) + b;
}


///////////// SINUSOIDAL EASING: sin(t) ///////////////
// sinusoidal easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in position, d: duration
double octaspire_easing_in_sine(double const t, double const b, double const c, double const d)
{
    return -c * cos(t/d * (M_PI/2)) + c + b;
}

// sinusoidal easing out - decelerating to zero velocity
double octaspire_easing_out_sine(double const t, double const b, double const c, double const d)
{
    return c * sin(t/d * (M_PI/2)) + b;
}

// sinusoidal easing in/out - accelerating until halfway, then decelerating
double octaspire_easing_in_out_sine(double const t, double const b, double const c, double const d)
{
    return -c/2 * (cos(M_PI*t/d) - 1) + b;
}


///////////// EXPONENTIAL EASING: 2^t /////////////////
// exponential easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in position, d: duration
double octaspire_easing_in_expo(double const t, double const b, double const c, double const d)
{
    return (t == 0) ? b : c * pow(2, 10 * (t/d - 1)) + b;
}

// exponential easing out - decelerating to zero velocity
double octaspire_easing_out_expo(double const t, double const b, double const c, double const d)
{
    return (t==d) ? b+c : c * (-pow(2, -10 * t/d) + 1) + b;
}

// exponential easing in/out - accelerating until halfway, then decelerating
double octaspire_easing_in_out_expo(double t, double const b, double const c, double const d)
{
    if (t==0) return b;
    if (t==d) return b+c;

    t /= (d * 0.5);

    if (t < 1) return c/2 * pow(2, 10 * (t - 1)) + b;

    --t;

    return c/2 * (-pow(2, -10 * t) + 2) + b;
}


/////////// CIRCULAR EASING: sqrt(1-t^2) //////////////
// circular easing in - accelerating from zero velocity
// t: current time, b: beginning value, c: change in position, d: duration
double octaspire_easing_in_circ(double t, double const b, double const c, double const d)
{
    t /= d;
    return -c * (sqrt(1 - t*t) - 1) + b;
}

// circular easing out - decelerating to zero velocity
double octaspire_easing_out_circ(double t, double const b, double const c, double const d)
{
    t = (t / d) -1;
    return c * sqrt(1 - (t*t)) + b;
}

// circular easing in/out - acceleration until halfway, then deceleration
double octaspire_easing_in_out_circ(double t, double const b, double const c, double const d)
{
    t /= (d * 0.5);

    if (t < 1) return -c/2 * (sqrt(1 - t*t) - 1) + b;
    t -= 2;
    return c/2 * (sqrt(1 - t*t) + 1) + b;
}


/////////// ELASTIC EASING: exponentially decaying sine wave  //////////////
// t: current time, b: beginning value, c: change in value, d: duration, a: amplitude (optional),
// p: period (optional)
// t and d can be in frames or seconds/milliseconds
double octaspire_easing_in_elastic(
    double t,
    double const b,
    double const c,
    double const d,
    double a,
    double p)
{
    double s = 0;

    if (t==0) return b;

    t /= d;

    if (t==1) return b+c;

    if (!p) p=d*0.3;


    if (a < fabs(c)) { a=c; s=p/4; }
    else s = p/(2*M_PI) * asin (c/a);

    t -= 1;
    return -(a*pow(2, 10*t) * sin( (t*d-s)*(2*M_PI)/p )) + b;
}

double octaspire_easing_out_elastic(
    double t,
    double const b,
    double const c,
    double const d,
    double a,
    double p)
{
    double s = 0;

    if (t==0) return b;

    t /= d;

    if (t==1) return b+c;

    if (!p) p=d*0.3;

    if (a < fabs(c)) { a=c; s=p/4; }
    else s = p/(2*M_PI) * asin (c/a);
    return a*pow(2, -10*t) * sin( (t*d-s)*(2*M_PI)/p ) + c + b;
}

double octaspire_easing_in_out_elastic(
    double t,
    double const b,
    double const c,
    double const d,
    double a,
    double p)
{
    double s = 0;

    if (t==0) return b;

    t /= (d * 0.5);

    if (t==2) return b+c;

    if (!p) p=d*(0.3*1.5);

    if (a < fabs(c)) { a=c; s=p/4; }
    else s = p/(2*M_PI) * asin (c/a);

    if (t < 1)
    {
        --t;
        return -.5*(a*pow(2, 10*t) * sin( (t*d-s)*(2*M_PI)/p )) + b;
    }

    --t;
    return a*pow(2, -10*t) * sin( (t*d-s)*(2*M_PI)/p )*.5 + c + b;
}


/////////// BACK EASING: overshooting cubic easing: (s+1)*t^3 - s*t^2  //////////////
// back easing in - backtracking slightly, then reversing direction and moving to target
// t: current time, b: beginning value, c: change in value, d: duration,
// s: overshoot amount (optional)
// t and d can be in frames or seconds/milliseconds
// s controls the amount of overshoot: higher s means greater overshoot
// s has a default value of 1.70158, which produces an overshoot of 10 percent
// s==0 produces cubic easing with no overshoot
double octaspire_easing_in_back(
    double t,
    double const b,
    double const c,
    double const d,
    double const s)
{
    t /= d;
    return c*t*t*((s+1)*t - s) + b;
}

// back easing out - moving towards target, overshooting it slightly, then reversing
// and coming back to target
double octaspire_easing_out_back(
    double t,
    double const b,
    double const c,
    double const d,
    double const s)
{
    t = (t / d) -1;
    return c*(t*t*((s+1)*t + s) + 1) + b;
}

// back easing in/out - backtracking slightly, then reversing direction and moving to target,
// then overshooting target, reversing, and finally coming back to target
double octaspire_easing_in_out_back(
    double t,
    double const b,
    double const c,
    double const d,
    double s)
{
    t /= (d * 0.5);
    s *= 1.525;

    if (t < 1)
    {
        return c/2*(t*t*((s+1)*t - s)) + b;
    }

    t -= 2;
    return c/2*(t*t*((s+1)*t + s) + 2) + b;
}


/////////// BOUNCE EASING: exponentially decaying parabolic bounce  //////////////
// bounce easing in
// t: current time, b: beginning value, c: change in position, d: duration
double octaspire_easing_in_bounce(double const t, double const b, double const c, double const d)
{
    return c - octaspire_easing_out_bounce (d-t, 0, c, d) + b;
}

// bounce easing out
double octaspire_easing_out_bounce(double t, double const b, double const c, double const d)
{
    t /= d;

    if (t < (1/2.75))
    {
        return c*(7.5625*t*t) + b;
    }
    else if (t < (2/2.75))
    {
        t-=(1.5/2.75);
        return c*(7.5625*t*t + 0.75) + b;
    }
    else if (t < (2.5/2.75))
    {
        t-=(2.25/2.75);
        return c*(7.5625*t*t + 0.9375) + b;
    }
    else
    {
        t-=(2.625/2.75);
        return c*(7.5625*t*t + 0.984375) + b;
    }
}

// bounce easing in/out
double octaspire_easing_in_out_bounce(
    double const t,
    double const b,
    double const c,
    double const d)
{
    if (t < d/2) return octaspire_easing_in_bounce (t*2, 0, c, d) * .5 + b;
    return octaspire_easing_out_bounce (t*2-d, 0, c, d) * .5 + c*.5 + b;
}
//////////////////////////////////easing ends///////////////////////////////////////////////


typedef enum octaspire_dern_ease_type_t
{
    OCTASPIRE_EASING_LINEAR,
    OCTASPIRE_EASING_IN_QUAD,
    OCTASPIRE_EASING_OUT_QUAD,
    OCTASPIRE_EASING_IN_OUT_QUAD,
    OCTASPIRE_EASING_IN_CUBIC,
    OCTASPIRE_EASING_OUT_CUBIC,
    OCTASPIRE_EASING_IN_OUT_CUBIC,
    OCTASPIRE_EASING_IN_QUART,
    OCTASPIRE_EASING_OUT_QUART,
    OCTASPIRE_EASING_IN_OUT_QUART,
    OCTASPIRE_EASING_IN_QUINT,
    OCTASPIRE_EASING_OUT_QUINT,
    OCTASPIRE_EASING_IN_OUT_QUINT,
    OCTASPIRE_EASING_IN_SINE,
    OCTASPIRE_EASING_OUT_SINE,
    OCTASPIRE_EASING_IN_OUT_SINE,
    OCTASPIRE_EASING_IN_EXPO,
    OCTASPIRE_EASING_OUT_EXPO,
    OCTASPIRE_EASING_IN_OUT_EXPO,
    OCTASPIRE_EASING_IN_CIRC,
    OCTASPIRE_EASING_OUT_CIRC,
    OCTASPIRE_EASING_IN_OUT_CIRC,
    OCTASPIRE_EASING_IN_ELASTIC,
    OCTASPIRE_EASING_OUT_ELASTIC,
    OCTASPIRE_EASING_IN_OUT_ELASTIC,
    OCTASPIRE_EASING_IN_BACK,
    OCTASPIRE_EASING_OUT_BACK,
    OCTASPIRE_EASING_IN_OUT_BACK,
    OCTASPIRE_EASING_IN_BOUNCE,
    OCTASPIRE_EASING_OUT_BOUNCE,
    OCTASPIRE_EASING_IN_OUT_BOUNCE
}
octaspire_dern_ease_type_t;

typedef struct octaspire_dern_ease_t
{
    octaspire_dern_ease_type_t          typeTag;
    double                              t;
    double                              b;
    double                              c;
    double                              d;
    double                              s;
    double                              a;
    double                              p;
    octaspire_dern_value_t *            targetValue;
    octaspire_container_utf8_string_t * evalOnDone;
    bool                                isDone;
}
octaspire_dern_ease_t;

static octaspire_container_vector_t * dern_easing_private_easings = 0;



octaspire_dern_value_t *dern_easing_add(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 5)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'easing-add' expects at least five arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_ease_t ease = {.targetValue = 0, .evalOnDone = 0, .isDone=false};

    octaspire_dern_value_t const * const typeNameArg =
        octaspire_dern_value_as_vector_get_element_of_type_at_const(
            arguments,
            OCTASPIRE_DERN_VALUE_TAG_SYMBOL,
            0);

    if (!typeNameArg)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'easing-add' expects symbol for the easing type as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(
                octaspire_dern_value_as_vector_get_element_type_at_const(arguments, 0)));
    }

    ease.targetValue = octaspire_dern_value_as_vector_get_element_at(arguments, 1);

    octaspire_helpers_verify_not_null(ease.targetValue);

    if (!octaspire_dern_value_is_number(ease.targetValue))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'easing-add' expects real or integer as the easing target value as "
            "the first argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(ease.targetValue->typeTag));
    }

    ease.t            = 0;

    octaspire_dern_value_t const * const beginValue =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(beginValue);

    if (!octaspire_dern_value_is_number(beginValue))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'easing-add' expects real or integer (begin) as the third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(beginValue->typeTag));
    }

    ease.b           = octaspire_dern_value_as_number_get_value(beginValue);

    octaspire_dern_value_t const * const changeValue =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(changeValue);

    if (!octaspire_dern_value_is_number(changeValue))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'easing-add' expects real or integer (change) as the fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(changeValue->typeTag));
    }

    ease.c           = octaspire_dern_value_as_number_get_value(changeValue);

    octaspire_dern_value_t const * const durationValue =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 4);

    octaspire_helpers_verify_not_null(durationValue);

    if (!octaspire_dern_value_is_number(durationValue))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'easing-add' expects real or integer (duration) as the fifth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(durationValue->typeTag));
    }

    ease.d           = octaspire_dern_value_as_number_get_value(durationValue);



    if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "linear"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five or six arguments if type is 'linear'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_LINEAR;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-quad"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five or six arguments if type is 'in-quad'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_QUAD;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-quad"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five or six arguments if type is 'out-quad'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_OUT_QUAD;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-quad"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'out-quad'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_OUT_QUAD;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-out-quad"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-out-quad'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_OUT_QUAD;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-cubic"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-cubic'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_CUBIC;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-cubic"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'out-cubic'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_OUT_CUBIC;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-out-cubic"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-out-cubic'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_OUT_CUBIC;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-quart"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-quart'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_QUART;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-quart"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'out-quart'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_OUT_QUART;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-out-quart"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-out-quart'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_OUT_QUART;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-quint"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-quint'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_QUINT;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-quint"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'out-quint'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_OUT_QUINT;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-out-quint"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-out-quint'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_OUT_QUINT;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-sine"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-sine'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_SINE;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-sine"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'out-sine'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_OUT_SINE;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-out-sine"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-out-sine'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_OUT_SINE;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. "
                    "Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-expo"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-expo'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_EXPO;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-expo"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'out-expo'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_OUT_EXPO;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-out-expo"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-out-expo'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_OUT_EXPO;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-circ"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-circ'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_CIRC;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-circ"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'out-circ'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_OUT_CIRC;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-out-circ"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-out-circ'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_OUT_CIRC;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-elastic"))
    {
        if (numArgs != 7 && numArgs != 8)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects seven arguments if type is 'in-elastic'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_ELASTIC;

        octaspire_dern_value_t const * const amplitudeValue =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 5);

        octaspire_helpers_verify_not_null(amplitudeValue);

        if (!octaspire_dern_value_is_number(amplitudeValue))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects real or integer (amplitude) as the sixth argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(amplitudeValue->typeTag));
        }

        ease.a = octaspire_dern_value_as_number_get_value(amplitudeValue);


        octaspire_dern_value_t const * const periodValue =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 6);

        octaspire_helpers_verify_not_null(periodValue);

        if (!octaspire_dern_value_is_number(periodValue))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects real or integer (period) as the seventh argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(periodValue->typeTag));
        }

        ease.p = octaspire_dern_value_as_number_get_value(periodValue);


        if (numArgs == 8)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-elastic"))
    {
        if (numArgs != 7 && numArgs != 8)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects sevin arguments if type is 'out-elastic'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_OUT_ELASTIC;

        octaspire_dern_value_t const * const amplitudeValue =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 5);

        octaspire_helpers_verify_not_null(amplitudeValue);

        if (!octaspire_dern_value_is_number(amplitudeValue))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects real or integer (amplitude) as the sixth argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(amplitudeValue->typeTag));
        }

        ease.a = octaspire_dern_value_as_number_get_value(amplitudeValue);


        octaspire_dern_value_t const * const periodValue =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 6);

        octaspire_helpers_verify_not_null(periodValue);

        if (!octaspire_dern_value_is_number(periodValue))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects real or integer (period) as the seventh argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(periodValue->typeTag));
        }

        ease.p = octaspire_dern_value_as_number_get_value(periodValue);


        if (numArgs == 8)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-out-elastic"))
    {
        if (numArgs != 7 && numArgs != 8)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects sevin arguments if type is 'in-out-elastic'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_OUT_ELASTIC;

        octaspire_dern_value_t const * const amplitudeValue =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 5);

        octaspire_helpers_verify_not_null(amplitudeValue);

        if (!octaspire_dern_value_is_number(amplitudeValue))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects real or integer (amplitude) as the sixth argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(amplitudeValue->typeTag));
        }

        ease.a = octaspire_dern_value_as_number_get_value(amplitudeValue);



        octaspire_dern_value_t const * const periodValue =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 6);

        octaspire_helpers_verify_not_null(periodValue);

        if (!octaspire_dern_value_is_number(periodValue))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects real or integer (period) as the seventh argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(periodValue->typeTag));
        }

        ease.p = octaspire_dern_value_as_number_get_value(periodValue);

        if (numArgs == 8)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-back"))
    {
        if (numArgs != 6 && numArgs != 7)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects six arguments if type is 'in-back'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_BACK;

        octaspire_dern_value_t const * const overshootValue =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 5);

        octaspire_helpers_verify_not_null(overshootValue);

        if (!octaspire_dern_value_is_number(overshootValue))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects real or integer (overshoot) as the sixth argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(overshootValue->typeTag));
        }

        ease.s = octaspire_dern_value_as_number_get_value(overshootValue);

        if (numArgs == 7)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-back"))
    {
        if (numArgs != 6 && numArgs != 7)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects six arguments if type is 'out-back'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_OUT_BACK;

        octaspire_dern_value_t const * const overshootValue =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 5);

        octaspire_helpers_verify_not_null(overshootValue);

        if (!octaspire_dern_value_is_number(overshootValue))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects real or integer (overshoot) as the sixth argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(overshootValue->typeTag));
        }

        ease.s = octaspire_dern_value_as_number_get_value(overshootValue);

        if (numArgs == 7)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-out-back"))
    {
        if (numArgs != 6 && numArgs != 7)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects six arguments if type is 'in-out-back'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_OUT_BACK;

        octaspire_dern_value_t const * const overshootValue =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 5);

        octaspire_helpers_verify_not_null(overshootValue);

        if (!octaspire_dern_value_is_number(overshootValue))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects real (overshoot) as the sixth argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(overshootValue->typeTag));
        }

        ease.s = octaspire_dern_value_as_number_get_value(overshootValue);

        if (numArgs == 7)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-bounce"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-bounce'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_BOUNCE;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "out-bounce"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'out-bounce'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_BOUNCE;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(typeNameArg, "in-out-bounce"))
    {
        if (numArgs != 5 && numArgs != 6)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'easing-add' expects five arguments if type is 'in-out-bounce'. "
                "%zu arguments were given.",
                numArgs);
        }

        ease.typeTag      = OCTASPIRE_EASING_IN_BOUNCE;

        if (numArgs == 6)
        {
            octaspire_dern_value_t const * const evalOnDoneVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, numArgs - 1);

            octaspire_helpers_verify_not_null(evalOnDoneVal);

            if (!octaspire_dern_value_is_text(evalOnDoneVal))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'easing-add' expects text (string or symbol) as the value to be "
                    "evaluated when easing is done. Type %s was given as %zu. argument.",
                    octaspire_dern_value_helper_get_type_as_c_string(evalOnDoneVal->typeTag),
                    numArgs);
            }

            ease.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_text_get_c_string(evalOnDoneVal),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(ease.evalOnDone);
        }
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'easing-add': unknown easing: '%s'.",
            octaspire_dern_value_as_symbol_get_c_string(typeNameArg));
    }

    if (!octaspire_container_vector_push_back_element(dern_easing_private_easings, &ease))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'easing-add' failed: cannot save easing.");
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_easing_update(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'easing-update' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }


    double dt = 0;


    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_real(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'easing-update' expects real dt as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    dt = octaspire_dern_value_as_number_get_value(firstArg);

    size_t i = 0;
    while (i < octaspire_container_vector_get_length(dern_easing_private_easings))
    {
        octaspire_dern_ease_t * const ease =
            (octaspire_dern_ease_t*)octaspire_container_vector_get_element_at(
                dern_easing_private_easings,
                i);

        octaspire_helpers_verify_not_null(ease);
        octaspire_helpers_verify_not_null(ease->targetValue);

        ease->t += dt;

        if (ease->t >= ease->d)
        {
            octaspire_dern_value_as_number_set_value(ease->targetValue, ease->b + ease->c);
            ease->isDone = true;

            if (ease->evalOnDone)
            {
                octaspire_dern_value_t const * const result =
                    octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                        vm,
                        octaspire_container_utf8_string_get_c_string(ease->evalOnDone));

                if (result && octaspire_dern_value_is_error(result))
                {
                    octaspire_dern_value_print(
                        result,
                        octaspire_dern_vm_get_allocator(vm));
                }
            }

            ++i;
        }
        else
        {
            switch (ease->typeTag)
            {
                case OCTASPIRE_EASING_LINEAR:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_linear(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_QUAD:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_quad(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_OUT_QUAD:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_out_quad(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_OUT_QUAD:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_out_quad(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_CUBIC:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_cubic(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_OUT_CUBIC:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_out_cubic(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_OUT_CUBIC:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_out_cubic(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_QUART:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_quart(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_OUT_QUART:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_out_quart(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_OUT_QUART:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_out_quart(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_QUINT:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_quint (ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_OUT_QUINT:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_out_quint(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_OUT_QUINT:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_out_quint(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_SINE:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_sine(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_OUT_SINE:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_out_sine(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_OUT_SINE:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_out_sine(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_EXPO:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_expo(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_OUT_EXPO:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_out_expo(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_OUT_EXPO:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_out_expo(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_CIRC:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_circ(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_OUT_CIRC:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_out_circ(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_OUT_CIRC:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_out_circ(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_ELASTIC:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_elastic(
                            ease->t,
                            ease->b,
                            ease->c,
                            ease->d,
                            ease->a,
                            ease->p));
                break;

                case OCTASPIRE_EASING_OUT_ELASTIC:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_out_elastic(
                            ease->t,
                            ease->b,
                            ease->c,
                            ease->d,
                            ease->a,
                            ease->p));
                break;

                case OCTASPIRE_EASING_IN_OUT_ELASTIC:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_out_elastic(
                            ease->t,
                            ease->b,
                            ease->c,
                            ease->d,
                            ease->a,
                            ease->p));
                break;

                case OCTASPIRE_EASING_IN_BACK:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_back(ease->t, ease->b, ease->c, ease->d, ease->s));
                break;

                case OCTASPIRE_EASING_OUT_BACK:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_out_back(ease->t, ease->b, ease->c, ease->d, ease->s));
                break;

                case OCTASPIRE_EASING_IN_OUT_BACK:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_out_back(ease->t, ease->b, ease->c, ease->d, ease->s));
                break;

                case OCTASPIRE_EASING_IN_BOUNCE:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_bounce(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_OUT_BOUNCE:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_out_bounce(ease->t, ease->b, ease->c, ease->d));
                break;

                case OCTASPIRE_EASING_IN_OUT_BOUNCE:
                    octaspire_dern_value_as_number_set_value(
                        ease->targetValue,
                        octaspire_easing_in_out_bounce(ease->t, ease->b, ease->c, ease->d));
                break;
            }

            ++i;
        }
    }

    i = 0;
    while (i < octaspire_container_vector_get_length(dern_easing_private_easings))
    {
        octaspire_dern_ease_t * const ease =
            (octaspire_dern_ease_t*)octaspire_container_vector_get_element_at(
                dern_easing_private_easings,
                i);

        octaspire_helpers_verify_not_null(ease);

        if (ease->isDone)
        {
            octaspire_container_vector_remove_element_at(dern_easing_private_easings, i);
        }
        else
        {
            ++i;
        }
    }


    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_easing_has_any(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'easing-has-any' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        !octaspire_container_vector_is_empty(dern_easing_private_easings));
}

bool dern_easing_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    dern_easing_private_easings = octaspire_container_vector_new(
        sizeof(octaspire_dern_ease_t),
        false,
        0,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_easing_private_easings)
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "easing-add",
            dern_easing_add,
            1,
            "(easing-add typename reguired arg1 arg2 .. argn) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "easing-update",
            dern_easing_update,
            1,
            "(easing-update dt) -> true",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "easing-has-any",
            dern_easing_has_any,
            1,
            "(easing-has-any) -> <true or false>",
            true,
            targetEnv))
    {
        return false;
    }

    return true;
}

bool dern_easing_mark_all(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    for (size_t i = 0; i < octaspire_container_vector_get_length(dern_easing_private_easings); ++i)
    {
        octaspire_dern_ease_t * const ease =
            (octaspire_dern_ease_t*)octaspire_container_vector_get_element_at(dern_easing_private_easings, i);

        octaspire_helpers_verify_not_null(ease);
        octaspire_helpers_verify_not_null(ease->targetValue);

        if (!octaspire_dern_value_mark(ease->targetValue))
        {
            return false;
        }
    }

    return true;
}

bool dern_easing_clean(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    octaspire_container_vector_release(dern_easing_private_easings);
    dern_easing_private_easings = 0;

    return true;
}

