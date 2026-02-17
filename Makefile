PLUGIN = ab_switch_midi
BUNDLE = ab-switch.lv2
CFLAGS = -fPIC -O2 -Wall
LV2DIR ?= /usr/lib/lv2

all: $(BUNDLE)/$(PLUGIN).so

$(BUNDLE)/$(PLUGIN).so: ab_switch_midi.c
	mkdir -p $(BUNDLE)
	gcc $(CFLAGS) -shared -o $@ $<
	cp *.ttl $(BUNDLE)/
install: all
	mkdir -p $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	cp -r $(BUNDLE)/* $(DESTDIR)$(LV2DIR)/$(BUNDLE)

clean:
	rm -rf $(BUNDLE) *.o *.so
