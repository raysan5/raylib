#ifndef AL_EFX_H
#define AL_EFX_H


#ifdef __cplusplus
extern "C" {
#endif

#define ALC_EXT_EFX_NAME                         "ALC_EXT_EFX"

#define ALC_EFX_MAJOR_VERSION                    0x20001
#define ALC_EFX_MINOR_VERSION                    0x20002
#define ALC_MAX_AUXILIARY_SENDS                  0x20003


/* Listener properties. */
#define AL_METERS_PER_UNIT                       0x20004

/* Source properties. */
#define AL_DIRECT_FILTER                         0x20005
#define AL_AUXILIARY_SEND_FILTER                 0x20006
#define AL_AIR_ABSORPTION_FACTOR                 0x20007
#define AL_ROOM_ROLLOFF_FACTOR                   0x20008
#define AL_CONE_OUTER_GAINHF                     0x20009
#define AL_DIRECT_FILTER_GAINHF_AUTO             0x2000A
#define AL_AUXILIARY_SEND_FILTER_GAIN_AUTO       0x2000B
#define AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO     0x2000C


/* Effect properties. */

/* Reverb effect parameters */
#define AL_REVERB_DENSITY                        0x0001
#define AL_REVERB_DIFFUSION                      0x0002
#define AL_REVERB_GAIN                           0x0003
#define AL_REVERB_GAINHF                         0x0004
#define AL_REVERB_DECAY_TIME                     0x0005
#define AL_REVERB_DECAY_HFRATIO                  0x0006
#define AL_REVERB_REFLECTIONS_GAIN               0x0007
#define AL_REVERB_REFLECTIONS_DELAY              0x0008
#define AL_REVERB_LATE_REVERB_GAIN               0x0009
#define AL_REVERB_LATE_REVERB_DELAY              0x000A
#define AL_REVERB_AIR_ABSORPTION_GAINHF          0x000B
#define AL_REVERB_ROOM_ROLLOFF_FACTOR            0x000C
#define AL_REVERB_DECAY_HFLIMIT                  0x000D

/* EAX Reverb effect parameters */
#define AL_EAXREVERB_DENSITY                     0x0001
#define AL_EAXREVERB_DIFFUSION                   0x0002
#define AL_EAXREVERB_GAIN                        0x0003
#define AL_EAXREVERB_GAINHF                      0x0004
#define AL_EAXREVERB_GAINLF                      0x0005
#define AL_EAXREVERB_DECAY_TIME                  0x0006
#define AL_EAXREVERB_DECAY_HFRATIO               0x0007
#define AL_EAXREVERB_DECAY_LFRATIO               0x0008
#define AL_EAXREVERB_REFLECTIONS_GAIN            0x0009
#define AL_EAXREVERB_REFLECTIONS_DELAY           0x000A
#define AL_EAXREVERB_REFLECTIONS_PAN             0x000B
#define AL_EAXREVERB_LATE_REVERB_GAIN            0x000C
#define AL_EAXREVERB_LATE_REVERB_DELAY           0x000D
#define AL_EAXREVERB_LATE_REVERB_PAN             0x000E
#define AL_EAXREVERB_ECHO_TIME                   0x000F
#define AL_EAXREVERB_ECHO_DEPTH                  0x0010
#define AL_EAXREVERB_MODULATION_TIME             0x0011
#define AL_EAXREVERB_MODULATION_DEPTH            0x0012
#define AL_EAXREVERB_AIR_ABSORPTION_GAINHF       0x0013
#define AL_EAXREVERB_HFREFERENCE                 0x0014
#define AL_EAXREVERB_LFREFERENCE                 0x0015
#define AL_EAXREVERB_ROOM_ROLLOFF_FACTOR         0x0016
#define AL_EAXREVERB_DECAY_HFLIMIT               0x0017

/* Chorus effect parameters */
#define AL_CHORUS_WAVEFORM                       0x0001
#define AL_CHORUS_PHASE                          0x0002
#define AL_CHORUS_RATE                           0x0003
#define AL_CHORUS_DEPTH                          0x0004
#define AL_CHORUS_FEEDBACK                       0x0005
#define AL_CHORUS_DELAY                          0x0006

/* Distortion effect parameters */
#define AL_DISTORTION_EDGE                       0x0001
#define AL_DISTORTION_GAIN                       0x0002
#define AL_DISTORTION_LOWPASS_CUTOFF             0x0003
#define AL_DISTORTION_EQCENTER                   0x0004
#define AL_DISTORTION_EQBANDWIDTH                0x0005

/* Echo effect parameters */
#define AL_ECHO_DELAY                            0x0001
#define AL_ECHO_LRDELAY                          0x0002
#define AL_ECHO_DAMPING                          0x0003
#define AL_ECHO_FEEDBACK                         0x0004
#define AL_ECHO_SPREAD                           0x0005

/* Flanger effect parameters */
#define AL_FLANGER_WAVEFORM                      0x0001
#define AL_FLANGER_PHASE                         0x0002
#define AL_FLANGER_RATE                          0x0003
#define AL_FLANGER_DEPTH                         0x0004
#define AL_FLANGER_FEEDBACK                      0x0005
#define AL_FLANGER_DELAY                         0x0006

/* Frequency shifter effect parameters */
#define AL_FREQUENCY_SHIFTER_FREQUENCY           0x0001
#define AL_FREQUENCY_SHIFTER_LEFT_DIRECTION      0x0002
#define AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION     0x0003

/* Vocal morpher effect parameters */
#define AL_VOCAL_MORPHER_PHONEMEA                0x0001
#define AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING  0x0002
#define AL_VOCAL_MORPHER_PHONEMEB                0x0003
#define AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING  0x0004
#define AL_VOCAL_MORPHER_WAVEFORM                0x0005
#define AL_VOCAL_MORPHER_RATE                    0x0006

/* Pitchshifter effect parameters */
#define AL_PITCH_SHIFTER_COARSE_TUNE             0x0001
#define AL_PITCH_SHIFTER_FINE_TUNE               0x0002

/* Ringmodulator effect parameters */
#define AL_RING_MODULATOR_FREQUENCY              0x0001
#define AL_RING_MODULATOR_HIGHPASS_CUTOFF        0x0002
#define AL_RING_MODULATOR_WAVEFORM               0x0003

/* Autowah effect parameters */
#define AL_AUTOWAH_ATTACK_TIME                   0x0001
#define AL_AUTOWAH_RELEASE_TIME                  0x0002
#define AL_AUTOWAH_RESONANCE                     0x0003
#define AL_AUTOWAH_PEAK_GAIN                     0x0004

/* Compressor effect parameters */
#define AL_COMPRESSOR_ONOFF                      0x0001

/* Equalizer effect parameters */
#define AL_EQUALIZER_LOW_GAIN                    0x0001
#define AL_EQUALIZER_LOW_CUTOFF                  0x0002
#define AL_EQUALIZER_MID1_GAIN                   0x0003
#define AL_EQUALIZER_MID1_CENTER                 0x0004
#define AL_EQUALIZER_MID1_WIDTH                  0x0005
#define AL_EQUALIZER_MID2_GAIN                   0x0006
#define AL_EQUALIZER_MID2_CENTER                 0x0007
#define AL_EQUALIZER_MID2_WIDTH                  0x0008
#define AL_EQUALIZER_HIGH_GAIN                   0x0009
#define AL_EQUALIZER_HIGH_CUTOFF                 0x000A

/* Effect type */
#define AL_EFFECT_FIRST_PARAMETER                0x0000
#define AL_EFFECT_LAST_PARAMETER                 0x8000
#define AL_EFFECT_TYPE                           0x8001

/* Effect types, used with the AL_EFFECT_TYPE property */
#define AL_EFFECT_NULL                           0x0000
#define AL_EFFECT_REVERB                         0x0001
#define AL_EFFECT_CHORUS                         0x0002
#define AL_EFFECT_DISTORTION                     0x0003
#define AL_EFFECT_ECHO                           0x0004
#define AL_EFFECT_FLANGER                        0x0005
#define AL_EFFECT_FREQUENCY_SHIFTER              0x0006
#define AL_EFFECT_VOCAL_MORPHER                  0x0007
#define AL_EFFECT_PITCH_SHIFTER                  0x0008
#define AL_EFFECT_RING_MODULATOR                 0x0009
#define AL_EFFECT_AUTOWAH                        0x000A
#define AL_EFFECT_COMPRESSOR                     0x000B
#define AL_EFFECT_EQUALIZER                      0x000C
#define AL_EFFECT_EAXREVERB                      0x8000

/* Auxiliary Effect Slot properties. */
#define AL_EFFECTSLOT_EFFECT                     0x0001
#define AL_EFFECTSLOT_GAIN                       0x0002
#define AL_EFFECTSLOT_AUXILIARY_SEND_AUTO        0x0003

/* NULL Auxiliary Slot ID to disable a source send. */
#define AL_EFFECTSLOT_NULL                       0x0000


/* Filter properties. */

/* Lowpass filter parameters */
#define AL_LOWPASS_GAIN                          0x0001
#define AL_LOWPASS_GAINHF                        0x0002

/* Highpass filter parameters */
#define AL_HIGHPASS_GAIN                         0x0001
#define AL_HIGHPASS_GAINLF                       0x0002

/* Bandpass filter parameters */
#define AL_BANDPASS_GAIN                         0x0001
#define AL_BANDPASS_GAINLF                       0x0002
#define AL_BANDPASS_GAINHF                       0x0003

/* Filter type */
#define AL_FILTER_FIRST_PARAMETER                0x0000
#define AL_FILTER_LAST_PARAMETER                 0x8000
#define AL_FILTER_TYPE                           0x8001

/* Filter types, used with the AL_FILTER_TYPE property */
#define AL_FILTER_NULL                           0x0000
#define AL_FILTER_LOWPASS                        0x0001
#define AL_FILTER_HIGHPASS                       0x0002
#define AL_FILTER_BANDPASS                       0x0003


/* Effect object function types. */
typedef void (AL_APIENTRY *LPALGENEFFECTS)(ALsizei, ALuint*);
typedef void (AL_APIENTRY *LPALDELETEEFFECTS)(ALsizei, ALuint*);
typedef ALboolean (AL_APIENTRY *LPALISEFFECT)(ALuint);
typedef void (AL_APIENTRY *LPALEFFECTI)(ALuint, ALenum, ALint);
typedef void (AL_APIENTRY *LPALEFFECTIV)(ALuint, ALenum, ALint*);
typedef void (AL_APIENTRY *LPALEFFECTF)(ALuint, ALenum, ALfloat);
typedef void (AL_APIENTRY *LPALEFFECTFV)(ALuint, ALenum, ALfloat*);
typedef void (AL_APIENTRY *LPALGETEFFECTI)(ALuint, ALenum, ALint*);
typedef void (AL_APIENTRY *LPALGETEFFECTIV)(ALuint, ALenum, ALint*);
typedef void (AL_APIENTRY *LPALGETEFFECTF)(ALuint, ALenum, ALfloat*);
typedef void (AL_APIENTRY *LPALGETEFFECTFV)(ALuint, ALenum, ALfloat*);

/* Filter object function types. */
typedef void (AL_APIENTRY *LPALGENFILTERS)(ALsizei, ALuint*);
typedef void (AL_APIENTRY *LPALDELETEFILTERS)(ALsizei, ALuint*);
typedef ALboolean (AL_APIENTRY *LPALISFILTER)(ALuint);
typedef void (AL_APIENTRY *LPALFILTERI)(ALuint, ALenum, ALint);
typedef void (AL_APIENTRY *LPALFILTERIV)(ALuint, ALenum, ALint*);
typedef void (AL_APIENTRY *LPALFILTERF)(ALuint, ALenum, ALfloat);
typedef void (AL_APIENTRY *LPALFILTERFV)(ALuint, ALenum, ALfloat*);
typedef void (AL_APIENTRY *LPALGETFILTERI)(ALuint, ALenum, ALint*);
typedef void (AL_APIENTRY *LPALGETFILTERIV)(ALuint, ALenum, ALint*);
typedef void (AL_APIENTRY *LPALGETFILTERF)(ALuint, ALenum, ALfloat*);
typedef void (AL_APIENTRY *LPALGETFILTERFV)(ALuint, ALenum, ALfloat*);

/* Auxiliary Effect Slot object function types. */
typedef void (AL_APIENTRY *LPALGENAUXILIARYEFFECTSLOTS)(ALsizei, ALuint*);
typedef void (AL_APIENTRY *LPALDELETEAUXILIARYEFFECTSLOTS)(ALsizei, ALuint*);
typedef ALboolean (AL_APIENTRY *LPALISAUXILIARYEFFECTSLOT)(ALuint);
typedef void (AL_APIENTRY *LPALAUXILIARYEFFECTSLOTI)(ALuint, ALenum, ALint);
typedef void (AL_APIENTRY *LPALAUXILIARYEFFECTSLOTIV)(ALuint, ALenum, ALint*);
typedef void (AL_APIENTRY *LPALAUXILIARYEFFECTSLOTF)(ALuint, ALenum, ALfloat);
typedef void (AL_APIENTRY *LPALAUXILIARYEFFECTSLOTFV)(ALuint, ALenum, ALfloat*);
typedef void (AL_APIENTRY *LPALGETAUXILIARYEFFECTSLOTI)(ALuint, ALenum, ALint*);
typedef void (AL_APIENTRY *LPALGETAUXILIARYEFFECTSLOTIV)(ALuint, ALenum, ALint*);
typedef void (AL_APIENTRY *LPALGETAUXILIARYEFFECTSLOTF)(ALuint, ALenum, ALfloat*);
typedef void (AL_APIENTRY *LPALGETAUXILIARYEFFECTSLOTFV)(ALuint, ALenum, ALfloat*);

#ifdef AL_ALEXT_PROTOTYPES
AL_API ALvoid AL_APIENTRY alGenEffects(ALsizei n, ALuint *effects);
AL_API ALvoid AL_APIENTRY alDeleteEffects(ALsizei n, ALuint *effects);
AL_API ALboolean AL_APIENTRY alIsEffect(ALuint effect);
AL_API ALvoid AL_APIENTRY alEffecti(ALuint effect, ALenum param, ALint iValue);
AL_API ALvoid AL_APIENTRY alEffectiv(ALuint effect, ALenum param, ALint *piValues);
AL_API ALvoid AL_APIENTRY alEffectf(ALuint effect, ALenum param, ALfloat flValue);
AL_API ALvoid AL_APIENTRY alEffectfv(ALuint effect, ALenum param, ALfloat *pflValues);
AL_API ALvoid AL_APIENTRY alGetEffecti(ALuint effect, ALenum param, ALint *piValue);
AL_API ALvoid AL_APIENTRY alGetEffectiv(ALuint effect, ALenum param, ALint *piValues);
AL_API ALvoid AL_APIENTRY alGetEffectf(ALuint effect, ALenum param, ALfloat *pflValue);
AL_API ALvoid AL_APIENTRY alGetEffectfv(ALuint effect, ALenum param, ALfloat *pflValues);

AL_API ALvoid AL_APIENTRY alGenFilters(ALsizei n, ALuint *filters);
AL_API ALvoid AL_APIENTRY alDeleteFilters(ALsizei n, ALuint *filters);
AL_API ALboolean AL_APIENTRY alIsFilter(ALuint filter);
AL_API ALvoid AL_APIENTRY alFilteri(ALuint filter, ALenum param, ALint iValue);
AL_API ALvoid AL_APIENTRY alFilteriv(ALuint filter, ALenum param, ALint *piValues);
AL_API ALvoid AL_APIENTRY alFilterf(ALuint filter, ALenum param, ALfloat flValue);
AL_API ALvoid AL_APIENTRY alFilterfv(ALuint filter, ALenum param, ALfloat *pflValues);
AL_API ALvoid AL_APIENTRY alGetFilteri(ALuint filter, ALenum param, ALint *piValue);
AL_API ALvoid AL_APIENTRY alGetFilteriv(ALuint filter, ALenum param, ALint *piValues);
AL_API ALvoid AL_APIENTRY alGetFilterf(ALuint filter, ALenum param, ALfloat *pflValue);
AL_API ALvoid AL_APIENTRY alGetFilterfv(ALuint filter, ALenum param, ALfloat *pflValues);

AL_API ALvoid AL_APIENTRY alGenAuxiliaryEffectSlots(ALsizei n, ALuint *effectslots);
AL_API ALvoid AL_APIENTRY alDeleteAuxiliaryEffectSlots(ALsizei n, ALuint *effectslots);
AL_API ALboolean AL_APIENTRY alIsAuxiliaryEffectSlot(ALuint effectslot);
AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint iValue);
AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotiv(ALuint effectslot, ALenum param, ALint *piValues);
AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat flValue);
AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotfv(ALuint effectslot, ALenum param, ALfloat *pflValues);
AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint *piValue);
AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotiv(ALuint effectslot, ALenum param, ALint *piValues);
AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat *pflValue);
AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotfv(ALuint effectslot, ALenum param, ALfloat *pflValues);
#endif

/* Filter ranges and defaults. */

/* Lowpass filter */
#define LOWPASS_MIN_GAIN                         (0.0f)
#define LOWPASS_MAX_GAIN                         (1.0f)
#define LOWPASS_DEFAULT_GAIN                     (1.0f)

#define LOWPASS_MIN_GAINHF                       (0.0f)
#define LOWPASS_MAX_GAINHF                       (1.0f)
#define LOWPASS_DEFAULT_GAINHF                   (1.0f)

/* Highpass filter */
#define HIGHPASS_MIN_GAIN                        (0.0f)
#define HIGHPASS_MAX_GAIN                        (1.0f)
#define HIGHPASS_DEFAULT_GAIN                    (1.0f)

#define HIGHPASS_MIN_GAINLF                      (0.0f)
#define HIGHPASS_MAX_GAINLF                      (1.0f)
#define HIGHPASS_DEFAULT_GAINLF                  (1.0f)

/* Bandpass filter */
#define BANDPASS_MIN_GAIN                        (0.0f)
#define BANDPASS_MAX_GAIN                        (1.0f)
#define BANDPASS_DEFAULT_GAIN                    (1.0f)

#define BANDPASS_MIN_GAINHF                      (0.0f)
#define BANDPASS_MAX_GAINHF                      (1.0f)
#define BANDPASS_DEFAULT_GAINHF                  (1.0f)

#define BANDPASS_MIN_GAINLF                      (0.0f)
#define BANDPASS_MAX_GAINLF                      (1.0f)
#define BANDPASS_DEFAULT_GAINLF                  (1.0f)


/* Effect parameter ranges and defaults. */

/* Standard reverb effect */
#define AL_REVERB_MIN_DENSITY                    (0.0f)
#define AL_REVERB_MAX_DENSITY                    (1.0f)
#define AL_REVERB_DEFAULT_DENSITY                (1.0f)

#define AL_REVERB_MIN_DIFFUSION                  (0.0f)
#define AL_REVERB_MAX_DIFFUSION                  (1.0f)
#define AL_REVERB_DEFAULT_DIFFUSION              (1.0f)

#define AL_REVERB_MIN_GAIN                       (0.0f)
#define AL_REVERB_MAX_GAIN                       (1.0f)
#define AL_REVERB_DEFAULT_GAIN                   (0.32f)

#define AL_REVERB_MIN_GAINHF                     (0.0f)
#define AL_REVERB_MAX_GAINHF                     (1.0f)
#define AL_REVERB_DEFAULT_GAINHF                 (0.89f)

#define AL_REVERB_MIN_DECAY_TIME                 (0.1f)
#define AL_REVERB_MAX_DECAY_TIME                 (20.0f)
#define AL_REVERB_DEFAULT_DECAY_TIME             (1.49f)

#define AL_REVERB_MIN_DECAY_HFRATIO              (0.1f)
#define AL_REVERB_MAX_DECAY_HFRATIO              (2.0f)
#define AL_REVERB_DEFAULT_DECAY_HFRATIO          (0.83f)

#define AL_REVERB_MIN_REFLECTIONS_GAIN           (0.0f)
#define AL_REVERB_MAX_REFLECTIONS_GAIN           (3.16f)
#define AL_REVERB_DEFAULT_REFLECTIONS_GAIN       (0.05f)

#define AL_REVERB_MIN_REFLECTIONS_DELAY          (0.0f)
#define AL_REVERB_MAX_REFLECTIONS_DELAY          (0.3f)
#define AL_REVERB_DEFAULT_REFLECTIONS_DELAY      (0.007f)

#define AL_REVERB_MIN_LATE_REVERB_GAIN           (0.0f)
#define AL_REVERB_MAX_LATE_REVERB_GAIN           (10.0f)
#define AL_REVERB_DEFAULT_LATE_REVERB_GAIN       (1.26f)

#define AL_REVERB_MIN_LATE_REVERB_DELAY          (0.0f)
#define AL_REVERB_MAX_LATE_REVERB_DELAY          (0.1f)
#define AL_REVERB_DEFAULT_LATE_REVERB_DELAY      (0.011f)

#define AL_REVERB_MIN_AIR_ABSORPTION_GAINHF      (0.892f)
#define AL_REVERB_MAX_AIR_ABSORPTION_GAINHF      (1.0f)
#define AL_REVERB_DEFAULT_AIR_ABSORPTION_GAINHF  (0.994f)

#define AL_REVERB_MIN_ROOM_ROLLOFF_FACTOR        (0.0f)
#define AL_REVERB_MAX_ROOM_ROLLOFF_FACTOR        (10.0f)
#define AL_REVERB_DEFAULT_ROOM_ROLLOFF_FACTOR    (0.0f)

#define AL_REVERB_MIN_DECAY_HFLIMIT              AL_FALSE
#define AL_REVERB_MAX_DECAY_HFLIMIT              AL_TRUE
#define AL_REVERB_DEFAULT_DECAY_HFLIMIT          AL_TRUE

/* EAX reverb effect */
#define AL_EAXREVERB_MIN_DENSITY                 (0.0f)
#define AL_EAXREVERB_MAX_DENSITY                 (1.0f)
#define AL_EAXREVERB_DEFAULT_DENSITY             (1.0f)

#define AL_EAXREVERB_MIN_DIFFUSION               (0.0f)
#define AL_EAXREVERB_MAX_DIFFUSION               (1.0f)
#define AL_EAXREVERB_DEFAULT_DIFFUSION           (1.0f)

#define AL_EAXREVERB_MIN_GAIN                    (0.0f)
#define AL_EAXREVERB_MAX_GAIN                    (1.0f)
#define AL_EAXREVERB_DEFAULT_GAIN                (0.32f)

#define AL_EAXREVERB_MIN_GAINHF                  (0.0f)
#define AL_EAXREVERB_MAX_GAINHF                  (1.0f)
#define AL_EAXREVERB_DEFAULT_GAINHF              (0.89f)

#define AL_EAXREVERB_MIN_GAINLF                  (0.0f)
#define AL_EAXREVERB_MAX_GAINLF                  (1.0f)
#define AL_EAXREVERB_DEFAULT_GAINLF              (1.0f)

#define AL_EAXREVERB_MIN_DECAY_TIME              (0.1f)
#define AL_EAXREVERB_MAX_DECAY_TIME              (20.0f)
#define AL_EAXREVERB_DEFAULT_DECAY_TIME          (1.49f)

#define AL_EAXREVERB_MIN_DECAY_HFRATIO           (0.1f)
#define AL_EAXREVERB_MAX_DECAY_HFRATIO           (2.0f)
#define AL_EAXREVERB_DEFAULT_DECAY_HFRATIO       (0.83f)

#define AL_EAXREVERB_MIN_DECAY_LFRATIO           (0.1f)
#define AL_EAXREVERB_MAX_DECAY_LFRATIO           (2.0f)
#define AL_EAXREVERB_DEFAULT_DECAY_LFRATIO       (1.0f)

#define AL_EAXREVERB_MIN_REFLECTIONS_GAIN        (0.0f)
#define AL_EAXREVERB_MAX_REFLECTIONS_GAIN        (3.16f)
#define AL_EAXREVERB_DEFAULT_REFLECTIONS_GAIN    (0.05f)

#define AL_EAXREVERB_MIN_REFLECTIONS_DELAY       (0.0f)
#define AL_EAXREVERB_MAX_REFLECTIONS_DELAY       (0.3f)
#define AL_EAXREVERB_DEFAULT_REFLECTIONS_DELAY   (0.007f)

#define AL_EAXREVERB_DEFAULT_REFLECTIONS_PAN_XYZ (0.0f)

#define AL_EAXREVERB_MIN_LATE_REVERB_GAIN        (0.0f)
#define AL_EAXREVERB_MAX_LATE_REVERB_GAIN        (10.0f)
#define AL_EAXREVERB_DEFAULT_LATE_REVERB_GAIN    (1.26f)

#define AL_EAXREVERB_MIN_LATE_REVERB_DELAY       (0.0f)
#define AL_EAXREVERB_MAX_LATE_REVERB_DELAY       (0.1f)
#define AL_EAXREVERB_DEFAULT_LATE_REVERB_DELAY   (0.011f)

#define AL_EAXREVERB_DEFAULT_LATE_REVERB_PAN_XYZ (0.0f)

#define AL_EAXREVERB_MIN_ECHO_TIME               (0.075f)
#define AL_EAXREVERB_MAX_ECHO_TIME               (0.25f)
#define AL_EAXREVERB_DEFAULT_ECHO_TIME           (0.25f)

#define AL_EAXREVERB_MIN_ECHO_DEPTH              (0.0f)
#define AL_EAXREVERB_MAX_ECHO_DEPTH              (1.0f)
#define AL_EAXREVERB_DEFAULT_ECHO_DEPTH          (0.0f)

#define AL_EAXREVERB_MIN_MODULATION_TIME         (0.04f)
#define AL_EAXREVERB_MAX_MODULATION_TIME         (4.0f)
#define AL_EAXREVERB_DEFAULT_MODULATION_TIME     (0.25f)

#define AL_EAXREVERB_MIN_MODULATION_DEPTH        (0.0f)
#define AL_EAXREVERB_MAX_MODULATION_DEPTH        (1.0f)
#define AL_EAXREVERB_DEFAULT_MODULATION_DEPTH    (0.0f)

#define AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF   (0.892f)
#define AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF   (1.0f)
#define AL_EAXREVERB_DEFAULT_AIR_ABSORPTION_GAINHF (0.994f)

#define AL_EAXREVERB_MIN_HFREFERENCE             (1000.0f)
#define AL_EAXREVERB_MAX_HFREFERENCE             (20000.0f)
#define AL_EAXREVERB_DEFAULT_HFREFERENCE         (5000.0f)

#define AL_EAXREVERB_MIN_LFREFERENCE             (20.0f)
#define AL_EAXREVERB_MAX_LFREFERENCE             (1000.0f)
#define AL_EAXREVERB_DEFAULT_LFREFERENCE         (250.0f)

#define AL_EAXREVERB_MIN_ROOM_ROLLOFF_FACTOR     (0.0f)
#define AL_EAXREVERB_MAX_ROOM_ROLLOFF_FACTOR     (10.0f)
#define AL_EAXREVERB_DEFAULT_ROOM_ROLLOFF_FACTOR (0.0f)

#define AL_EAXREVERB_MIN_DECAY_HFLIMIT           AL_FALSE
#define AL_EAXREVERB_MAX_DECAY_HFLIMIT           AL_TRUE
#define AL_EAXREVERB_DEFAULT_DECAY_HFLIMIT       AL_TRUE

/* Chorus effect */
#define AL_CHORUS_WAVEFORM_SINUSOID              (0)
#define AL_CHORUS_WAVEFORM_TRIANGLE              (1)

#define AL_CHORUS_MIN_WAVEFORM                   (0)
#define AL_CHORUS_MAX_WAVEFORM                   (1)
#define AL_CHORUS_DEFAULT_WAVEFORM               (1)

#define AL_CHORUS_MIN_PHASE                      (-180)
#define AL_CHORUS_MAX_PHASE                      (180)
#define AL_CHORUS_DEFAULT_PHASE                  (90)

#define AL_CHORUS_MIN_RATE                       (0.0f)
#define AL_CHORUS_MAX_RATE                       (10.0f)
#define AL_CHORUS_DEFAULT_RATE                   (1.1f)

#define AL_CHORUS_MIN_DEPTH                      (0.0f)
#define AL_CHORUS_MAX_DEPTH                      (1.0f)
#define AL_CHORUS_DEFAULT_DEPTH                  (0.1f)

#define AL_CHORUS_MIN_FEEDBACK                   (-1.0f)
#define AL_CHORUS_MAX_FEEDBACK                   (1.0f)
#define AL_CHORUS_DEFAULT_FEEDBACK               (0.25f)

#define AL_CHORUS_MIN_DELAY                      (0.0f)
#define AL_CHORUS_MAX_DELAY                      (0.016f)
#define AL_CHORUS_DEFAULT_DELAY                  (0.016f)

/* Distortion effect */
#define AL_DISTORTION_MIN_EDGE                   (0.0f)
#define AL_DISTORTION_MAX_EDGE                   (1.0f)
#define AL_DISTORTION_DEFAULT_EDGE               (0.2f)

#define AL_DISTORTION_MIN_GAIN                   (0.01f)
#define AL_DISTORTION_MAX_GAIN                   (1.0f)
#define AL_DISTORTION_DEFAULT_GAIN               (0.05f)

#define AL_DISTORTION_MIN_LOWPASS_CUTOFF         (80.0f)
#define AL_DISTORTION_MAX_LOWPASS_CUTOFF         (24000.0f)
#define AL_DISTORTION_DEFAULT_LOWPASS_CUTOFF     (8000.0f)

#define AL_DISTORTION_MIN_EQCENTER               (80.0f)
#define AL_DISTORTION_MAX_EQCENTER               (24000.0f)
#define AL_DISTORTION_DEFAULT_EQCENTER           (3600.0f)

#define AL_DISTORTION_MIN_EQBANDWIDTH            (80.0f)
#define AL_DISTORTION_MAX_EQBANDWIDTH            (24000.0f)
#define AL_DISTORTION_DEFAULT_EQBANDWIDTH        (3600.0f)

/* Echo effect */
#define AL_ECHO_MIN_DELAY                        (0.0f)
#define AL_ECHO_MAX_DELAY                        (0.207f)
#define AL_ECHO_DEFAULT_DELAY                    (0.1f)

#define AL_ECHO_MIN_LRDELAY                      (0.0f)
#define AL_ECHO_MAX_LRDELAY                      (0.404f)
#define AL_ECHO_DEFAULT_LRDELAY                  (0.1f)

#define AL_ECHO_MIN_DAMPING                      (0.0f)
#define AL_ECHO_MAX_DAMPING                      (0.99f)
#define AL_ECHO_DEFAULT_DAMPING                  (0.5f)

#define AL_ECHO_MIN_FEEDBACK                     (0.0f)
#define AL_ECHO_MAX_FEEDBACK                     (1.0f)
#define AL_ECHO_DEFAULT_FEEDBACK                 (0.5f)

#define AL_ECHO_MIN_SPREAD                       (-1.0f)
#define AL_ECHO_MAX_SPREAD                       (1.0f)
#define AL_ECHO_DEFAULT_SPREAD                   (-1.0f)

/* Flanger effect */
#define AL_FLANGER_WAVEFORM_SINUSOID             (0)
#define AL_FLANGER_WAVEFORM_TRIANGLE             (1)

#define AL_FLANGER_MIN_WAVEFORM                  (0)
#define AL_FLANGER_MAX_WAVEFORM                  (1)
#define AL_FLANGER_DEFAULT_WAVEFORM              (1)

#define AL_FLANGER_MIN_PHASE                     (-180)
#define AL_FLANGER_MAX_PHASE                     (180)
#define AL_FLANGER_DEFAULT_PHASE                 (0)

#define AL_FLANGER_MIN_RATE                      (0.0f)
#define AL_FLANGER_MAX_RATE                      (10.0f)
#define AL_FLANGER_DEFAULT_RATE                  (0.27f)

#define AL_FLANGER_MIN_DEPTH                     (0.0f)
#define AL_FLANGER_MAX_DEPTH                     (1.0f)
#define AL_FLANGER_DEFAULT_DEPTH                 (1.0f)

#define AL_FLANGER_MIN_FEEDBACK                  (-1.0f)
#define AL_FLANGER_MAX_FEEDBACK                  (1.0f)
#define AL_FLANGER_DEFAULT_FEEDBACK              (-0.5f)

#define AL_FLANGER_MIN_DELAY                     (0.0f)
#define AL_FLANGER_MAX_DELAY                     (0.004f)
#define AL_FLANGER_DEFAULT_DELAY                 (0.002f)

/* Frequency shifter effect */
#define AL_FREQUENCY_SHIFTER_MIN_FREQUENCY       (0.0f)
#define AL_FREQUENCY_SHIFTER_MAX_FREQUENCY       (24000.0f)
#define AL_FREQUENCY_SHIFTER_DEFAULT_FREQUENCY   (0.0f)

#define AL_FREQUENCY_SHIFTER_MIN_LEFT_DIRECTION  (0)
#define AL_FREQUENCY_SHIFTER_MAX_LEFT_DIRECTION  (2)
#define AL_FREQUENCY_SHIFTER_DEFAULT_LEFT_DIRECTION (0)

#define AL_FREQUENCY_SHIFTER_DIRECTION_DOWN      (0)
#define AL_FREQUENCY_SHIFTER_DIRECTION_UP        (1)
#define AL_FREQUENCY_SHIFTER_DIRECTION_OFF       (2)

#define AL_FREQUENCY_SHIFTER_MIN_RIGHT_DIRECTION (0)
#define AL_FREQUENCY_SHIFTER_MAX_RIGHT_DIRECTION (2)
#define AL_FREQUENCY_SHIFTER_DEFAULT_RIGHT_DIRECTION (0)

/* Vocal morpher effect */
#define AL_VOCAL_MORPHER_MIN_PHONEMEA            (0)
#define AL_VOCAL_MORPHER_MAX_PHONEMEA            (29)
#define AL_VOCAL_MORPHER_DEFAULT_PHONEMEA        (0)

#define AL_VOCAL_MORPHER_MIN_PHONEMEA_COARSE_TUNING (-24)
#define AL_VOCAL_MORPHER_MAX_PHONEMEA_COARSE_TUNING (24)
#define AL_VOCAL_MORPHER_DEFAULT_PHONEMEA_COARSE_TUNING (0)

#define AL_VOCAL_MORPHER_MIN_PHONEMEB            (0)
#define AL_VOCAL_MORPHER_MAX_PHONEMEB            (29)
#define AL_VOCAL_MORPHER_DEFAULT_PHONEMEB        (10)

#define AL_VOCAL_MORPHER_MIN_PHONEMEB_COARSE_TUNING (-24)
#define AL_VOCAL_MORPHER_MAX_PHONEMEB_COARSE_TUNING (24)
#define AL_VOCAL_MORPHER_DEFAULT_PHONEMEB_COARSE_TUNING (0)

#define AL_VOCAL_MORPHER_PHONEME_A               (0)
#define AL_VOCAL_MORPHER_PHONEME_E               (1)
#define AL_VOCAL_MORPHER_PHONEME_I               (2)
#define AL_VOCAL_MORPHER_PHONEME_O               (3)
#define AL_VOCAL_MORPHER_PHONEME_U               (4)
#define AL_VOCAL_MORPHER_PHONEME_AA              (5)
#define AL_VOCAL_MORPHER_PHONEME_AE              (6)
#define AL_VOCAL_MORPHER_PHONEME_AH              (7)
#define AL_VOCAL_MORPHER_PHONEME_AO              (8)
#define AL_VOCAL_MORPHER_PHONEME_EH              (9)
#define AL_VOCAL_MORPHER_PHONEME_ER              (10)
#define AL_VOCAL_MORPHER_PHONEME_IH              (11)
#define AL_VOCAL_MORPHER_PHONEME_IY              (12)
#define AL_VOCAL_MORPHER_PHONEME_UH              (13)
#define AL_VOCAL_MORPHER_PHONEME_UW              (14)
#define AL_VOCAL_MORPHER_PHONEME_B               (15)
#define AL_VOCAL_MORPHER_PHONEME_D               (16)
#define AL_VOCAL_MORPHER_PHONEME_F               (17)
#define AL_VOCAL_MORPHER_PHONEME_G               (18)
#define AL_VOCAL_MORPHER_PHONEME_J               (19)
#define AL_VOCAL_MORPHER_PHONEME_K               (20)
#define AL_VOCAL_MORPHER_PHONEME_L               (21)
#define AL_VOCAL_MORPHER_PHONEME_M               (22)
#define AL_VOCAL_MORPHER_PHONEME_N               (23)
#define AL_VOCAL_MORPHER_PHONEME_P               (24)
#define AL_VOCAL_MORPHER_PHONEME_R               (25)
#define AL_VOCAL_MORPHER_PHONEME_S               (26)
#define AL_VOCAL_MORPHER_PHONEME_T               (27)
#define AL_VOCAL_MORPHER_PHONEME_V               (28)
#define AL_VOCAL_MORPHER_PHONEME_Z               (29)

#define AL_VOCAL_MORPHER_WAVEFORM_SINUSOID       (0)
#define AL_VOCAL_MORPHER_WAVEFORM_TRIANGLE       (1)
#define AL_VOCAL_MORPHER_WAVEFORM_SAWTOOTH       (2)

#define AL_VOCAL_MORPHER_MIN_WAVEFORM            (0)
#define AL_VOCAL_MORPHER_MAX_WAVEFORM            (2)
#define AL_VOCAL_MORPHER_DEFAULT_WAVEFORM        (0)

#define AL_VOCAL_MORPHER_MIN_RATE                (0.0f)
#define AL_VOCAL_MORPHER_MAX_RATE                (10.0f)
#define AL_VOCAL_MORPHER_DEFAULT_RATE            (1.41f)

/* Pitch shifter effect */
#define AL_PITCH_SHIFTER_MIN_COARSE_TUNE         (-12)
#define AL_PITCH_SHIFTER_MAX_COARSE_TUNE         (12)
#define AL_PITCH_SHIFTER_DEFAULT_COARSE_TUNE     (12)

#define AL_PITCH_SHIFTER_MIN_FINE_TUNE           (-50)
#define AL_PITCH_SHIFTER_MAX_FINE_TUNE           (50)
#define AL_PITCH_SHIFTER_DEFAULT_FINE_TUNE       (0)

/* Ring modulator effect */
#define AL_RING_MODULATOR_MIN_FREQUENCY          (0.0f)
#define AL_RING_MODULATOR_MAX_FREQUENCY          (8000.0f)
#define AL_RING_MODULATOR_DEFAULT_FREQUENCY      (440.0f)

#define AL_RING_MODULATOR_MIN_HIGHPASS_CUTOFF    (0.0f)
#define AL_RING_MODULATOR_MAX_HIGHPASS_CUTOFF    (24000.0f)
#define AL_RING_MODULATOR_DEFAULT_HIGHPASS_CUTOFF (800.0f)

#define AL_RING_MODULATOR_SINUSOID               (0)
#define AL_RING_MODULATOR_SAWTOOTH               (1)
#define AL_RING_MODULATOR_SQUARE                 (2)

#define AL_RING_MODULATOR_MIN_WAVEFORM           (0)
#define AL_RING_MODULATOR_MAX_WAVEFORM           (2)
#define AL_RING_MODULATOR_DEFAULT_WAVEFORM       (0)

/* Autowah effect */
#define AL_AUTOWAH_MIN_ATTACK_TIME               (0.0001f)
#define AL_AUTOWAH_MAX_ATTACK_TIME               (1.0f)
#define AL_AUTOWAH_DEFAULT_ATTACK_TIME           (0.06f)

#define AL_AUTOWAH_MIN_RELEASE_TIME              (0.0001f)
#define AL_AUTOWAH_MAX_RELEASE_TIME              (1.0f)
#define AL_AUTOWAH_DEFAULT_RELEASE_TIME          (0.06f)

#define AL_AUTOWAH_MIN_RESONANCE                 (2.0f)
#define AL_AUTOWAH_MAX_RESONANCE                 (1000.0f)
#define AL_AUTOWAH_DEFAULT_RESONANCE             (1000.0f)

#define AL_AUTOWAH_MIN_PEAK_GAIN                 (0.00003f)
#define AL_AUTOWAH_MAX_PEAK_GAIN                 (31621.0f)
#define AL_AUTOWAH_DEFAULT_PEAK_GAIN             (11.22f)

/* Compressor effect */
#define AL_COMPRESSOR_MIN_ONOFF                  (0)
#define AL_COMPRESSOR_MAX_ONOFF                  (1)
#define AL_COMPRESSOR_DEFAULT_ONOFF              (1)

/* Equalizer effect */
#define AL_EQUALIZER_MIN_LOW_GAIN                (0.126f)
#define AL_EQUALIZER_MAX_LOW_GAIN                (7.943f)
#define AL_EQUALIZER_DEFAULT_LOW_GAIN            (1.0f)

#define AL_EQUALIZER_MIN_LOW_CUTOFF              (50.0f)
#define AL_EQUALIZER_MAX_LOW_CUTOFF              (800.0f)
#define AL_EQUALIZER_DEFAULT_LOW_CUTOFF          (200.0f)

#define AL_EQUALIZER_MIN_MID1_GAIN               (0.126f)
#define AL_EQUALIZER_MAX_MID1_GAIN               (7.943f)
#define AL_EQUALIZER_DEFAULT_MID1_GAIN           (1.0f)

#define AL_EQUALIZER_MIN_MID1_CENTER             (200.0f)
#define AL_EQUALIZER_MAX_MID1_CENTER             (3000.0f)
#define AL_EQUALIZER_DEFAULT_MID1_CENTER         (500.0f)

#define AL_EQUALIZER_MIN_MID1_WIDTH              (0.01f)
#define AL_EQUALIZER_MAX_MID1_WIDTH              (1.0f)
#define AL_EQUALIZER_DEFAULT_MID1_WIDTH          (1.0f)

#define AL_EQUALIZER_MIN_MID2_GAIN               (0.126f)
#define AL_EQUALIZER_MAX_MID2_GAIN               (7.943f)
#define AL_EQUALIZER_DEFAULT_MID2_GAIN           (1.0f)

#define AL_EQUALIZER_MIN_MID2_CENTER             (1000.0f)
#define AL_EQUALIZER_MAX_MID2_CENTER             (8000.0f)
#define AL_EQUALIZER_DEFAULT_MID2_CENTER         (3000.0f)

#define AL_EQUALIZER_MIN_MID2_WIDTH              (0.01f)
#define AL_EQUALIZER_MAX_MID2_WIDTH              (1.0f)
#define AL_EQUALIZER_DEFAULT_MID2_WIDTH          (1.0f)

#define AL_EQUALIZER_MIN_HIGH_GAIN               (0.126f)
#define AL_EQUALIZER_MAX_HIGH_GAIN               (7.943f)
#define AL_EQUALIZER_DEFAULT_HIGH_GAIN           (1.0f)

#define AL_EQUALIZER_MIN_HIGH_CUTOFF             (4000.0f)
#define AL_EQUALIZER_MAX_HIGH_CUTOFF             (16000.0f)
#define AL_EQUALIZER_DEFAULT_HIGH_CUTOFF         (6000.0f)


/* Source parameter value ranges and defaults. */
#define AL_MIN_AIR_ABSORPTION_FACTOR             (0.0f)
#define AL_MAX_AIR_ABSORPTION_FACTOR             (10.0f)
#define AL_DEFAULT_AIR_ABSORPTION_FACTOR         (0.0f)

#define AL_MIN_ROOM_ROLLOFF_FACTOR               (0.0f)
#define AL_MAX_ROOM_ROLLOFF_FACTOR               (10.0f)
#define AL_DEFAULT_ROOM_ROLLOFF_FACTOR           (0.0f)

#define AL_MIN_CONE_OUTER_GAINHF                 (0.0f)
#define AL_MAX_CONE_OUTER_GAINHF                 (1.0f)
#define AL_DEFAULT_CONE_OUTER_GAINHF             (1.0f)

#define AL_MIN_DIRECT_FILTER_GAINHF_AUTO         AL_FALSE
#define AL_MAX_DIRECT_FILTER_GAINHF_AUTO         AL_TRUE
#define AL_DEFAULT_DIRECT_FILTER_GAINHF_AUTO     AL_TRUE

#define AL_MIN_AUXILIARY_SEND_FILTER_GAIN_AUTO   AL_FALSE
#define AL_MAX_AUXILIARY_SEND_FILTER_GAIN_AUTO   AL_TRUE
#define AL_DEFAULT_AUXILIARY_SEND_FILTER_GAIN_AUTO AL_TRUE

#define AL_MIN_AUXILIARY_SEND_FILTER_GAINHF_AUTO AL_FALSE
#define AL_MAX_AUXILIARY_SEND_FILTER_GAINHF_AUTO AL_TRUE
#define AL_DEFAULT_AUXILIARY_SEND_FILTER_GAINHF_AUTO AL_TRUE


/* Listener parameter value ranges and defaults. */
#define AL_MIN_METERS_PER_UNIT                   FLT_MIN
#define AL_MAX_METERS_PER_UNIT                   FLT_MAX
#define AL_DEFAULT_METERS_PER_UNIT               (1.0f)


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* AL_EFX_H */
