//**************************************************************************#
//                                                                         	#
// libVision.c - mruby testing                                				# 
// Copyright (C) 2016 Amedeo Setti,                                        	#
// amedeo[dot]setti[at]unitn.it                                            	#
// Original gem by Paolo Bosetti at                                        	#
// https://github.com/UniTN-Mechatronics/mruby-raspicam                    	#
// paolo[dot]bosetti[at]unitn.it                                           	#
// Department of Industrial Engineering, University of Trento              	#
//                                                                         	#
// This library is free software.  You can redistribute it and/or          	#
// modify it under the terms of the GNU GENERAL PUBLIC LICENSE 2.0.        	#
//                                                                         	#
// This library is distributed in the hope that it will be useful,         	#
// but WITHOUT ANY WARRANTY; without even the implied warranty of          	#
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           	#
// Artistic License 2.0 for more details.                                  	#
//                                                                         	#
// See the file LICENSE                                                    	#
//                                                                         	#
//**************************************************************************#

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "mruby.h"
#include "mruby/variable.h"
#include "mruby/string.h"
#include "mruby/data.h"
#include "mruby/class.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/numeric.h"

#include "libVision.h"

#define MARK_LINE
#define E_LIBVISION_ERROR (mrb_class_get(mrb, "LibVisionError"))

// Struct holding data:
typedef struct { CLibVision_ptr libvision; } libvision_data_s;

// Garbage collector handler, for libvision_data struct
// if libvision_data contains other dynamic data, free it too!
// Check it with GC.start
static void libvision_data_destructor(mrb_state *mrb, void *p_) {
	libvision_data_s *pd = (libvision_data_s *)p_;
  	CLibVision_deinit(pd->libvision);
  	free(pd);
};

// Creating data type and reference for GC, in a const struct
const struct mrb_data_type libvision_data_type = {"libvision_data",
                                                 libvision_data_destructor};

// Utility function for getting the struct out of the wrapping IV @data
static void mrb_libvision_get_data(mrb_state *mrb, mrb_value self, libvision_data_s **data) {
	mrb_value data_value;
  	data_value = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@data"));

  	// Loading data from data_value into p_data:
  	Data_Get_Struct(mrb, data_value, &libvision_data_type, *data);
  	if (!*data)
    	mrb_raise(mrb, E_RUNTIME_ERROR, "Could not access @data");
}

// Data Initializer C function (not exposed!)
static void mrb_libvision_init(mrb_state *mrb, mrb_value self) {
	mrb_value data_value;    // this IV holds the data
  	libvision_data_s *p_data; // pointer to the C struct
  	data_value = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@data"));

  	// if @data already exists, free its content:
  	if (!mrb_nil_p(data_value)) {
    	Data_Get_Struct(mrb, data_value, &libvision_data_type, p_data);
    	free(p_data);
  	}
  	// Allocate and zero-out the data struct:
  	p_data = (libvision_data_s *)malloc(sizeof(libvision_data_s));

  	// memset(p_data, 0, sizeof(raspicam_data_s));
  	if (!p_data)
    	mrb_raise(mrb, E_RUNTIME_ERROR, "Could not allocate @data");

  	// Wrap struct into @data:
  	mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@data"), // set @data
             mrb_obj_value( // with value hold in struct
                 Data_Wrap_Struct(mrb, mrb->object_class, &libvision_data_type,
                                  p_data)));

    // Now set values into struct:
    p_data->libvision = CLibVision_init();
}

static mrb_value mrb_libvision_initialize(mrb_state *mrb, mrb_value self) {
  	// Call strcut initializer:
  	mrb_libvision_init(mrb, self);
    return mrb_nil_value();
}

static void mrb_libvision_execute(mrb_state *mrb, mrb_value self) {
  	/*libvision_data_s *p_data = NULL;
  	// call utility for unwrapping @data into p_data:
  	mrb_array functions;
	mrb_int size;
   	mrb_get_args(mrb, "ai", &functions, &size);
   	// call utility for unwrapping @data into p_data:
    mrb_libvision_get_data(mrb, self, &p_data);  
  	CLibVision_requireOperations(p_data->libvision, functions, size);*/
}

void mrb_mruby_libvision_gem_init(mrb_state *mrb) {
  	struct RClass *libvision;
  	mrb_define_class(mrb, "LibVisionError", mrb_class_get(mrb, "Exception"));
  	libvision = mrb_define_class(mrb, "LibVision", mrb->object_class);

  	mrb_define_method(mrb, libvision, "initialize", mrb_libvision_initialize, MRB_ARGS_NONE());
    
  	mrb_define_method(mrb, libvision, "execute", mrb_libvision_execute, MRB_ARGS_REQ(2));
}

void mrb_mruby_libvision_gem_final(mrb_state *mrb) {}
