#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lv2/core/lv2.h"
#include "lv2/midi/midi.h"
#include "lv2/urid/urid.h"
#include "lv2/atom/atom.h"
#include "lv2/atom/util.h"

#define AB_URI "http://example.org/plugins/ab-switch"

typedef enum {
  IN_A_L  = 0,
  IN_A_R  = 1,
  IN_B_L  = 2,
  IN_B_R  = 3,
  OUT_L_A = 4,
  OUT_R_A = 5,
  OUT_L_B = 6,
  OUT_R_B = 7,
  MIDI_IN = 8,
  CC_NUM  = 9
} PortIndex;

typedef struct {
  const float* in_a_l;
  const float* in_a_r;
  const float* in_b_l;
  const float* in_b_r;
  float*       out_l_a;
  float*       out_r_a;
  float*       out_l_b;
  float*       out_r_b;
  const LV2_Atom_Sequence* midi_in;
  const float* cc_number;

  int state;
  float fade;
} AB;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features) {
  AB* self = (AB*)calloc(1, sizeof(AB));
  self->state = 0;
  self->fade = 0.0f;
  return (LV2_Handle)self;
}

static void
connect_port(LV2_Handle instance, uint32_t port, void* data) {
  AB* self = (AB*)instance;
  switch ((PortIndex)port) {
  case IN_A_L:  self->in_a_l = (const float*)data; break;
  case IN_A_R:  self->in_a_r = (const float*)data; break;
  case IN_B_L:  self->in_b_l = (const float*)data; break;
  case IN_B_R:  self->in_b_r = (const float*)data; break;
  case OUT_L_A: self->out_l_a = (float*)data; break;
  case OUT_R_A: self->out_r_a = (float*)data; break;
  case OUT_L_B: self->out_l_b = (float*)data; break;
  case OUT_R_B: self->out_r_b = (float*)data; break;
  case MIDI_IN: self->midi_in = (const LV2_Atom_Sequence*)data; break;
  case CC_NUM:  self->cc_number = (const float*)data; break;
  }
}

static void
run(LV2_Handle instance, uint32_t n_samples) {
  AB* self = (AB*)instance;

  // Vérifier MIDI
  if (self->midi_in) {
    LV2_ATOM_SEQUENCE_FOREACH(self->midi_in, ev) {
      const uint8_t* const msg = (const uint8_t*)(ev + 1);
      if ((msg[0] & 0xF0) == LV2_MIDI_MSG_CONTROLLER) {  // 0xB0
	uint8_t cc  = msg[1];
	uint8_t val = msg[2];
	uint8_t expected = (uint8_t)(*self->cc_number);
	if (cc == expected) {
	  self->state = (val >= 64) ? 1 : 0;
	}
      }
    }
  }

  // Crossfade simple
  float target = self->state ? 1.0f : 0.0f;
  float step = (target - self->fade) / (float)n_samples;

  for (uint32_t i = 0; i < n_samples; ++i) {
    self->fade += step;

    float aL = self->in_a_l ? self->in_a_l[i] : 0.0f;
    float aR = self->in_a_r ? self->in_a_r[i] : 0.0f;
    float bL = self->in_b_l ? self->in_b_l[i] : 0.0f;
    float bR = self->in_b_r ? self->in_b_r[i] : 0.0f;

    float gainA = 1.0f - self->fade;
    float gainB = self->fade;

    self->out_l_a[i] = aL * gainA;
    self->out_r_a[i] = aR * gainA;
    self->out_l_b[i] = bL * gainB;
    self->out_r_b[i] = bR * gainB;
  }
}

static void
cleanup(LV2_Handle instance) {
  free(instance);
}

static const LV2_Descriptor descriptor = {
  AB_URI,
  instantiate,
  connect_port,
  NULL,
  run,
  NULL,
  cleanup,
  NULL
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
  return (index == 0) ? &descriptor : NULL;
}
