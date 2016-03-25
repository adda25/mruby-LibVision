//**************************************************************************#
//                                                                         	#
// libvision.c - mruby testing                                              #
// Copyright (C) 2016 Amedeo Setti,                                         #
// amedeo[dot]setti[at]unitn.it                                             #
// Original project by Paolo Bosetti at                                     #
// https://github.com/UniTN-Mechatronics/mruby-raspicam                     #
// paolo[dot]bosetti[at]unitn.it                                            #
// Department of Industrial Engineering, University of Trento               #
//                                                                         	#
// This library is free software.  You can redistribute it and/or           #
// modify it under the terms of the GNU GENERAL PUBLIC LICENSE 2.0.         #
//                                                                         	#
// This library is distributed in the hope that it will be useful,          #
// but WITHOUT ANY WARRANTY; without even the implied warranty of           #
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
// Artistic License 2.0 for more details.                                   #
//                                                                         	#
// See the file LICENSE                                                     #
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
  // mrb_free(mrb, CLibVision_params(pd->libvision));
  CLibVision_deinit(pd->libvision);
  free(pd);
};

// Creating data type and reference for GC, in a const struct
const struct mrb_data_type libvision_data_type = {"libvision_data",
                                                  libvision_data_destructor};

// Utility function for getting the struct out of the wrapping IV @data
static void mrb_libvision_get_data(mrb_state *mrb, mrb_value self,
                                   libvision_data_s **data) {
  mrb_value data_value;
  data_value = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@data"));

  // Loading data from data_value into p_data:
  Data_Get_Struct(mrb, data_value, &libvision_data_type, *data);
  if (!*data)
    mrb_raise(mrb, E_RUNTIME_ERROR, "Could not access @data");
}

// Data Initializer C function (not exposed!)
static void mrb_libvision_init(mrb_state *mrb, mrb_value self) {
  mrb_value data_value;     // this IV holds the data
  libvision_data_s *p_data; // pointer to the C struct
  data_value = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@data"));

  // if @data already exists, free its content:
  if (!mrb_nil_p(data_value)) {
    Data_Get_Struct(mrb, data_value, &libvision_data_type, p_data);
    free(p_data);
  }
  // Allocate and zero-out the data struct:
  p_data = (libvision_data_s *)malloc(sizeof(libvision_data_s));

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

static mrb_value mrb_libvision_execute(mrb_state *mrb, mrb_value self) {
  mrb_value ary_in = mrb_nil_value();
  libvision_data_s *p_data = NULL;
  mrb_int i, arr_size;
  char **functions = NULL;
  mrb_get_args(mrb, "A", &ary_in);

  // call utility for unwrapping @data into p_data:
  mrb_libvision_get_data(mrb, self, &p_data);

  arr_size = RARRAY_LEN(ary_in);
  functions = mrb_calloc(mrb, arr_size, sizeof(char *));
  for (i = 0; i < arr_size; i++) {
    mrb_value elem = mrb_ary_ref(mrb, ary_in, i);
    if (mrb_string_p(elem)) {
      // int arena_idx = mrb_gc_arena_save(mrb);
      // mrb_gc_arena_restore(mrb, arena_idx);
      functions[i] = mrb_string_value_cstr(mrb, &elem);
      ;
    } else {
      mrb_raisef(mrb, E_RUNTIME_ERROR, "Non-string entry at position %S", elem);
    }
  }
  CLibVision_requireOperations(p_data->libvision, functions, arr_size);
  mrb_free(mrb, functions);
  return self;
}

/*
 ____                               ____  _                   _
|  _ \ __ _ _ __ __ _ _ __ ___  ___/ ___|| |_ _ __ _   _  ___| |_
| |_) / _` | '__/ _` | '_ ` _ \/ __\___ \| __| '__| | | |/ __| __|
|  __/ (_| | | | (_| | | | | | \__ \___) | |_| |  | |_| | (__| |_
|_|   \__,_|_|  \__,_|_| |_| |_|___/____/ \__|_|   \__,_|\___|\__|

*/

static mrb_value mrb_libvision_set_value_for_key(mrb_state *mrb,
                                                 mrb_value self) {
  mrb_value ary_in = mrb_nil_value();

  libvision_data_s *p_data = NULL;
  mrb_get_args(mrb, "A", &ary_in);
  mrb_libvision_get_data(mrb, self, &p_data);

  // Fetch key
  char *key;
  mrb_value elem = mrb_ary_entry(ary_in, 0);
  if (mrb_string_p(elem)) {
    key = mrb_string_value_cstr(mrb, &elem);
  } else {
    printf("RETURNING: \n");
    return self;
  }
  // Fetch values
  if (strcmp(key, "imagePath") == 0) {
    mrb_value mvalue = mrb_ary_entry(ary_in, 1);
    if (mrb_string_p(mvalue)) {
      static char imagePath[256];
      strcpy(imagePath, mrb_string_value_cstr(mrb, &mvalue));
      LibVisionParams *clvParams_ptr = CLibVision_params(p_data->libvision);
      clvParams_ptr->imagePath = imagePath;
    }
  } else if (strcmp(key, "savedImagePath") == 0) {
    mrb_value mvalue = mrb_ary_entry(ary_in, 1);
    if (mrb_string_p(mvalue)) {
      static char savedImagePath[256];
      strcpy(savedImagePath, mrb_string_value_cstr(mrb, &mvalue));
      LibVisionParams *clvParams_ptr = CLibVision_params(p_data->libvision);
      clvParams_ptr->savedImagePath = savedImagePath;
    }
  } else if (strcmp(key, "patternImagePath") == 0) {
    mrb_value mvalue = mrb_ary_entry(ary_in, 1);
    if (mrb_string_p(mvalue)) {
      LibVisionParams *clvParams_ptr = CLibVision_params(p_data->libvision);
      clvParams_ptr->patternImagePath = mrb_string_value_cstr(mrb, &mvalue);
    }
  } else if (strcmp(key, "colorRange") == 0) {
    int arr_size = RARRAY_LEN(ary_in);
    if (arr_size > 7) {
      return self;
    }
    for (int i = 1; i < arr_size; i++) {
      mrb_value elem = mrb_ary_entry(ary_in, i);
      if (mrb_fixnum_p(elem)) {
        CLibVision_params(p_data->libvision)->colorRange[i - 1] =
            mrb_to_flo(mrb, elem);
      }
    }
  } else if (strcmp(key, "cameraFrameSize") == 0) {
    mrb_value mvalue = mrb_ary_entry(ary_in, 1);
    if (mrb_fixnum_p(mvalue)) {
      mrb_int value = mrb_to_flo(mrb, mvalue);
      CLibVision_params(p_data->libvision)->cameraFrameSize[0] = (int)value;
    }
    mvalue = mrb_ary_entry(ary_in, 2);
    if (mrb_fixnum_p(mvalue)) {
      mrb_int value = mrb_to_flo(mrb, mvalue);
      CLibVision_params(p_data->libvision)->cameraFrameSize[1] = (int)value;
    }
  } else if (strcmp(key, "otsuThresh") == 0) {
    mrb_value mvalue = mrb_ary_entry(ary_in, 1);
    if (mrb_fixnum_p(mvalue)) {
      mrb_int value = mrb_to_flo(mrb, mvalue);
      CLibVision_params(p_data->libvision)->otsuThresh = (int)value;
    }
  } else if (strcmp(key, "adptThreshSize") == 0) {
    mrb_value mvalue = mrb_ary_entry(ary_in, 1);
    if (mrb_fixnum_p(mvalue)) {
      mrb_int value = mrb_to_flo(mrb, mvalue);
      CLibVision_params(p_data->libvision)->adptThreshSize = (int)value;
    }
  } else if (strcmp(key, "adptThreshMean") == 0) {
    mrb_value mvalue = mrb_ary_entry(ary_in, 1);
    if (mrb_fixnum_p(mvalue)) {
      mrb_int value = mrb_to_flo(mrb, mvalue);
      CLibVision_params(p_data->libvision)->adptThreshMean = (int)value;
    }
  }
  return self;
}

static mrb_value mrb_libvision_get_value_for_key(mrb_state *mrb,
                                                 mrb_value self) {
  mrb_value ary_in;
  libvision_data_s *p_data = NULL;
  mrb_get_args(mrb, "A", &ary_in); // TODO: Switch to string
  mrb_libvision_get_data(mrb, self, &p_data);

  // Fetch key
  char *key;
  mrb_value elem = mrb_ary_entry(ary_in, 0);
  if (mrb_string_p(elem)) {
    key = mrb_str_to_cstr(mrb, elem);
  } else {
    return self;
  }

  // Fetch values
  if (strcmp(key, "imagePath") == 0) {
    mrb_value ot =
        mrb_str_new_cstr(mrb, CLibVision_params(p_data->libvision)->imagePath);
    return ot;
  } else if (strcmp(key, "savedImagePath") == 0) {
    mrb_value ot = mrb_str_new_cstr(
        mrb, CLibVision_params(p_data->libvision)->savedImagePath);
    return ot;
  } else if (strcmp(key, "patternImagePath") == 0) {
    mrb_value ot = mrb_str_new_cstr(
        mrb, CLibVision_params(p_data->libvision)->patternImagePath);
    return ot;
  } else if (strcmp(key, "colorRange") == 0) {
    mrb_value ary_color = mrb_ary_new_capa(mrb, 6);
    for (int i = 0; i < 6; i++) {
      mrb_value ot =
          mrb_fixnum_value(CLibVision_params(p_data->libvision)->colorRange[i]);
      mrb_ary_push(mrb, ary_color, ot);
    }
    return ary_color;
  } else if (strcmp(key, "cameraFrameSize") == 0) {
    mrb_value ary_cam = mrb_ary_new_capa(mrb, 2);
    for (int i = 0; i < 2; i++) {
      mrb_value ot = mrb_fixnum_value(
          CLibVision_params(p_data->libvision)->cameraFrameSize[i]);
      mrb_ary_push(mrb, ary_cam, ot);
    }
    return ary_cam;
  } else if (strcmp(key, "otsuThresh") == 0) {
    mrb_int ot = CLibVision_params(p_data->libvision)->otsuThresh;
    return mrb_fixnum_value(ot);
  } else if (strcmp(key, "adptThreshSize") == 0) {
    mrb_int ot = CLibVision_params(p_data->libvision)->adptThreshSize;
    return mrb_fixnum_value(ot);
  } else if (strcmp(key, "adptThreshMean") == 0) {
    mrb_int ot = CLibVision_params(p_data->libvision)->adptThreshMean;
    return mrb_fixnum_value(ot);
  } else if (strcmp(key, "polygonsFounds") == 0) {
    mrb_int pf_size = CLibVision_params(p_data->libvision)->polygonsFounds;
    mrb_value pf_founds = mrb_ary_new_capa(mrb, pf_size);
    for (int i = 0; i < pf_size; i++) {
      mrb_int spf_size =
          CLibVision_params(p_data->libvision)->polygons[i].numberOfPoints;
      mrb_value spf_founds = mrb_ary_new_capa(mrb, spf_size);
      for (int j = 0; j < spf_size; j++) {
        mrb_value points = mrb_ary_new_capa(mrb, 2);
        mrb_value px = mrb_fixnum_value(
            CLibVision_params(p_data->libvision)->polygons[i].polyPoints[j].x);
        mrb_value py = mrb_fixnum_value(
            CLibVision_params(p_data->libvision)->polygons[i].polyPoints[j].y);
        mrb_ary_push(mrb, points, px);
        mrb_ary_push(mrb, points, py);
        mrb_ary_push(mrb, spf_founds, points);
      }
      mrb_ary_push(mrb, pf_founds, spf_founds);
    }
    return pf_founds;
  }
  return self;
}

static mrb_value mrb_libvision_test_debug(mrb_state *mrb, mrb_value self) {
  return self;
}
/*
 ___       _ _    ____
|_ _|_ __ (_) |_ / ___| ___ _ __ ___
 | || '_ \| | __| |  _ / _ \ '_ ` _ \
 | || | | | | |_| |_| |  __/ | | | | |
|___|_| |_|_|\__|\____|\___|_| |_| |_|

*/

void mrb_mruby_libvision_gem_init(mrb_state *mrb) {
  struct RClass *libvision;
  mrb_define_class(mrb, "LibVisionError", mrb_class_get(mrb, "Exception"));
  libvision = mrb_define_class(mrb, "LibVision", mrb->object_class);

  mrb_define_method(mrb, libvision, "initialize", mrb_libvision_initialize,
                    MRB_ARGS_NONE());

  mrb_define_method(mrb, libvision, "execute", mrb_libvision_execute,
                    MRB_ARGS_REQ(1));

  mrb_define_method(mrb, libvision, "set_value4key",
                    mrb_libvision_set_value_for_key, MRB_ARGS_REQ(1));

  mrb_define_method(mrb, libvision, "get_value4key",
                    mrb_libvision_get_value_for_key, MRB_ARGS_REQ(1));

  mrb_define_method(mrb, libvision, "testDebug", mrb_libvision_test_debug,
                    MRB_ARGS_NONE());
}

void mrb_mruby_libvision_gem_final(mrb_state *mrb) {}
