# NXP audio amplifier - TFA9896

TFA_VERSION              =tfa9896
#TFADSP_32BITS            =0
#TFADSP_DSP_MSG_APR_PACKET_STRATEGY=0
TFADSP_DSP_BUFFER_POOL   =1
TFA_SRC_DIR              =sound/soc/codecs/$(TFA_VERSION)
TFA98XX_FLAGS           := -DUSE_TFA9896

# cc flags
#ccflags-y               := -DDEBUG
#ccflags-y               += -I$(TFA_SRC_DIR)/inc
ccflags-y               := -I$(TFA_SRC_DIR)/inc
ccflags-y               += -Werror
#ifdef TFADSP_32BITS
#ccflags-y               += -DTFADSP_32BITS
#endif
#ifdef TFADSP_DSP_MSG_APR_PACKET_STRATEGY
#ccflags-y               += -DTFADSP_DSP_MSG_APR_PACKET_STRATEGY
#endif
ifdef TFADSP_DSP_BUFFER_POOL
ccflags-y               += -DTFADSP_DSP_BUFFER_POOL
endif

# objs
snd-soc-tfa98xx-objs    := tfa98xx.o
snd-soc-tfa98xx-objs    += tfa_container.o
snd-soc-tfa98xx-objs    += tfa_dsp.o
snd-soc-tfa98xx-objs    += tfa_init.o
snd-soc-tfa98xx-objs    += tfa_debug.o

# CFLAGS
CFLAGS_tfa98xx.o       += $(TFA98XX_FLAGS)
CFLAGS_tfa_container.o += $(TFA98XX_FLAGS)
CFLAGS_tfa_dsp.o       += $(TFA98XX_FLAGS)
CFLAGS_tfa_init.o      += $(TFA98XX_FLAGS)
CFLAGS_tfa_debug.o     += $(TFA98XX_FLAGS)

# built-in driver
obj-y	+= snd-soc-tfa98xx.o
