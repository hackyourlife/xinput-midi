#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <mmsystem.h>
#include <xinput.h>

#define	TYPE_NONE	0x80
#define	TYPE_PRESSURE	0x81
#define	TYPE_PITCH	0x82

typedef union {
	unsigned int word;
	unsigned char data[4];
} SHORTMSG;

typedef struct {
	char		devname[64];
	unsigned char	channel;
	unsigned char	thumb_l_x;
	unsigned char	thumb_l_y;
	unsigned char	thumb_r_x;
	unsigned char	thumb_r_y;
	unsigned char	left_trigger;
	unsigned char	right_trigger;
	unsigned char	dpad_up;
	unsigned char	dpad_down;
	unsigned char	dpad_left;
	unsigned char	dpad_right;
	unsigned char	start;
	unsigned char	back;
	unsigned char	left_thumb;
	unsigned char	right_thumb;
	unsigned char	left_shoulder;
	unsigned char	right_shoulder;
	unsigned char	a;
	unsigned char	b;
	unsigned char	x;
	unsigned char	y;
} CONFIG;

/* string functions */
char* ltrim(char* s, const char* sep)
{
	size_t trim;
	if(!sep) {
		sep = "\t\n\v\f\r ";
	}

	trim = strspn(s, sep);

	return s + trim;
}

char* rtrim(char* s, const char* sep)
{
	ssize_t i;
	if(!sep) {
		sep = "\t\n\v\f\r ";
	}

	i = strlen(s) - 1;
	while(i >= 0 && strchr(sep, s[i]) != NULL) {
		s[i] = 0;
		i--;
	}
	return s;
}

char* trim(char* s, const char* sep)
{
	return ltrim(rtrim(s, sep), sep);
}

/* config handling */
int CONFIGParseAssignment(unsigned char* option, const char* value)
{
	if(!strcmp("NONE", value)) {
		*option = TYPE_NONE;
	} else if(!strcmp("PITCH", value)) {
		*option = TYPE_PITCH;
	} else if(!strcmp("PRESSURE", value)) {
		*option = TYPE_PRESSURE;
	} else if(*value >= '0' && *value <= '9') {
		*option = atoi(value);
	} else {
		printf("Unsupported value \"%s\"\n", value);
		return 1;
	}

	return 0;
}

#define	PARSEASSIGN(name) \
	} else if(!strcmp(#name, option)) { \
		return CONFIGParseAssignment(&conf->name, value); \

int CONFIGParseOption(CONFIG* conf, const char* option, const char* value)
{
	if(!strcmp("device", option)) {
		strncpy(conf->devname, value, 64);
	} else if(!strcmp("channel", option)) {
		conf->channel = atoi(value) - 1;
		if(conf->channel >= 16) {
			printf("Invalid channel \"%s\"\n", value);
			conf->channel = 0;
		}
	PARSEASSIGN(thumb_l_x)
	PARSEASSIGN(thumb_l_y)
	PARSEASSIGN(thumb_r_x)
	PARSEASSIGN(thumb_r_y)
	PARSEASSIGN(left_trigger)
	PARSEASSIGN(right_trigger)
	PARSEASSIGN(dpad_up)
	PARSEASSIGN(dpad_down)
	PARSEASSIGN(dpad_left)
	PARSEASSIGN(dpad_right)
	PARSEASSIGN(start)
	PARSEASSIGN(back)
	PARSEASSIGN(left_thumb)
	PARSEASSIGN(right_thumb)
	PARSEASSIGN(left_shoulder)
	PARSEASSIGN(right_shoulder)
	PARSEASSIGN(a)
	PARSEASSIGN(b)
	PARSEASSIGN(x)
	PARSEASSIGN(y)
	} else {
		printf("Unknown option \"%s\"\n", option);
		return 1;
	}

	return 0;
}

#define	BUFSIZE	8192
int CONFIGRead(CONFIG* conf, const char* filename)
{
	char* line = (char*) malloc(BUFSIZE);

	FILE* f = fopen(filename, "rt");
	if(!f) {
		printf("Error opening file \"%s\"\n", filename);
		return 1;
	}

	/* reset config */
	conf->devname[0] = 0;
	conf->channel = 0;
	conf->thumb_l_x = TYPE_NONE;
	conf->thumb_l_y = TYPE_NONE;
	conf->thumb_r_x = TYPE_NONE;
	conf->thumb_r_y = TYPE_NONE;
	conf->left_trigger = TYPE_NONE;
	conf->right_trigger = TYPE_NONE;
	conf->dpad_up = TYPE_NONE;
	conf->dpad_down = TYPE_NONE;
	conf->dpad_left = TYPE_NONE;
	conf->dpad_right = TYPE_NONE;
	conf->start = TYPE_NONE;
	conf->back = TYPE_NONE;
	conf->left_thumb = TYPE_NONE;
	conf->right_thumb = TYPE_NONE;
	conf->left_shoulder = TYPE_NONE;
	conf->right_shoulder = TYPE_NONE;
	conf->a = TYPE_NONE;
	conf->b = TYPE_NONE;
	conf->x = TYPE_NONE;
	conf->y = TYPE_NONE;

	while(fgets(line, BUFSIZE, f)) {
		/* process line */

		char* comment = strchr(line, '#');
		char* s;
		char* equal;

		if(comment) {
			*comment = 0;
		}

		s = trim(line, NULL);

		if(!*s) {
			/* empty line */
			continue;
		}

		equal = strchr(s, '=');
		if(equal) {
			const char* option;
			const char* value;

			*equal = 0;
			s = trim(s, "\t\v\f ");
			if(!*s) {
				printf("Missing option name\n");
				continue;
			}

			option = s;
			value = trim(equal + 1, NULL);

			CONFIGParseOption(conf, option, value);
		} else {
			printf("Syntax error: missing '='\n");
		}
	}

	fclose(f);

	if(line) {
		free(line);
	}

	return 0;
}

void MIDIPrintDeviceList(void)
{
	MIDIOUTCAPS moc;
	unsigned long devcnt;
	unsigned long i;

	devcnt = midiOutGetNumDevs();
	printf("%lu MIDI devices:\n", devcnt);

	for(i = 0; i < devcnt; i++) {
		if(!midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS))) {
			printf("Device ID %lu: %s\n", i, moc.szPname);
		}
	}
}

int MIDIGetOutDeviceByName(const char* name)
{
	MIDIOUTCAPS moc;
	unsigned long devcnt;
	unsigned long i;

	devcnt = midiOutGetNumDevs();

	for(i = 0; i < devcnt; i++) {
		if(!midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS))) {
			if(!strcmp(moc.szPname, name)) {
				return i;
			}
		}
	}

	return -1;
}

void MIDISendCC(HMIDIOUT device, unsigned char channel, unsigned char cc,
		unsigned char value)
{
	SHORTMSG msg;
	msg.data[0] = 0xB0 | (channel & 0x0F);
	msg.data[1] = cc;
	msg.data[2] = value;
	msg.data[3] = 0;

	if(midiOutShortMsg(device, msg.word) != MMSYSERR_NOERROR) {
		printf("Failed to send MIDI message\n");
	}
}

void MIDISendPressure(HMIDIOUT device, unsigned char channel,
		unsigned char value)
{
	SHORTMSG msg;
	msg.data[0] = 0xD0 | (channel & 0x0F);
	msg.data[1] = value;
	msg.data[2] = 0;
	msg.data[3] = 0;

	if(midiOutShortMsg(device, msg.word) != MMSYSERR_NOERROR) {
		printf("Failed to send MIDI message\n");
	}
}

void MIDISendPitchBend(HMIDIOUT device, unsigned char channel, int value)
{
	SHORTMSG msg;
	unsigned int raw = value + 0x2000;

	if(value > 0x1FFF) {
		raw = 0x3FFF;
	} else if(value < -0x1FFF) {
		raw = 0;
	}

	msg.data[0] = 0xE0 | (channel & 0x0F);
	msg.data[1] = raw & 0x7F;
	msg.data[2] = (raw >> 7) & 0x7F;
	msg.data[3] = 0;

	if(midiOutShortMsg(device, msg.word) != MMSYSERR_NOERROR) {
		printf("Failed to send MIDI message\n");
	}
}

void PROCESSTrigger(HMIDIOUT device, unsigned char channel,
		unsigned char type, unsigned char value)
{
	switch(type) {
		case TYPE_NONE:
			return;
		case TYPE_PITCH:
			MIDISendPitchBend(device, channel, (((int) value) << 5) + 0x2000);
			break;
		case TYPE_PRESSURE:
			MIDISendPressure(device, channel, value >> 1);
			break;
		default:
			MIDISendCC(device, channel, type, value >> 1);
	}
}

void PROCESSStick(HMIDIOUT device, unsigned char channel,
		unsigned char type, signed short value)
{
	switch(type) {
		case TYPE_NONE:
			return;
		case TYPE_PITCH:
			MIDISendPitchBend(device, channel, value >> 2);
			break;
		case TYPE_PRESSURE:
			MIDISendPressure(device, channel, (value >> 9) + 0x40);
			break;
		default:
			MIDISendCC(device, channel, type, (value >> 9) + 0x40);
	}
}

void PROCESSButton(HMIDIOUT device, unsigned char channel,
		unsigned char type, int value)
{
	switch(type) {
		case TYPE_NONE:
		case TYPE_PITCH:
		case TYPE_PRESSURE:
			return;
		default:
			MIDISendCC(device, channel, type, value ? 0x7F : 0x00);
	}
}

int main(int argc, char** argv)
{
	CONFIG conf;
	HMIDIOUT device;
	int port;
	XINPUT_STATE state[XUSER_MAX_COUNT];
	int valid[XUSER_MAX_COUNT];

	if(argc != 2) {
		printf("Usage: %s config.cfg\n\n", *argv);

		MIDIPrintDeviceList();

		return 0;
	}

	if(CONFIGRead(&conf, argv[1])) {
		printf("Failed to load configuration\n");
		return 1;
	}

	port = MIDIGetOutDeviceByName(conf.devname);
	if(port == -1) {
		printf("Cannot find MIDI port \"%s\"\n", conf.devname);
		return 1;
	}

	printf("Opening MIDI port %d (%s)...\n", port, conf.devname);

	/* open MIDI device */
	if(midiOutOpen(&device, port, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
		printf("Error opening MIDI port\n");
		return 1;
	}

	/* reset controller data */
	memset(state, 0, sizeof(state));
	memset(valid, 0, sizeof(valid));

	printf("Press ESC or Q to quit.\n");
	printf("Waiting for events...\n");
	while(1) {
		unsigned int i;

		/* check for keyboard inputs */
		if(kbhit()) {
			int key = getch();
			if(key == 'q' || key == 'Q' || key == 0x1B) {
				break;
			}
		}

		/* poll controllers */
		for(i = 0; i < XUSER_MAX_COUNT; i++) {
			XINPUT_STATE current;
			/* poll controller */
			memset(&current, 0, sizeof(XINPUT_STATE));
			if(XInputGetState(i, &current) != ERROR_SUCCESS) {
				if(valid[i]) {
					printf("Lost controller %d\n", i);
				}
				valid[i] = 0;
				memset(&state[i], 0, sizeof(XINPUT_STATE));
				continue;
			}

			/* did anything change? */
			if((state[i].dwPacketNumber == current.dwPacketNumber) && !valid[i]) {
				/* no */
				continue;
			}

			if(!valid[i]) {
				printf("New controller: %d\n", i);
			}

			valid[i] = 1;

			if(state[i].Gamepad.bLeftTrigger != current.Gamepad.bLeftTrigger) {
				PROCESSTrigger(device, conf.channel, conf.left_trigger, current.Gamepad.bLeftTrigger);
			}
			if(state[i].Gamepad.bRightTrigger != current.Gamepad.bRightTrigger) {
				PROCESSTrigger(device, conf.channel, conf.right_trigger, current.Gamepad.bRightTrigger);
			}
			if(state[i].Gamepad.sThumbLX != current.Gamepad.sThumbLX) {
				PROCESSStick(device, conf.channel, conf.thumb_l_x, current.Gamepad.sThumbLX);
			}
			if(state[i].Gamepad.sThumbLY != current.Gamepad.sThumbLY) {
				PROCESSStick(device, conf.channel, conf.thumb_l_y, current.Gamepad.sThumbLY);
			}
			if(state[i].Gamepad.sThumbRX != current.Gamepad.sThumbRX) {
				PROCESSStick(device, conf.channel, conf.thumb_r_x, current.Gamepad.sThumbRX);
			}
			if(state[i].Gamepad.sThumbRY != current.Gamepad.sThumbRY) {
				PROCESSStick(device, conf.channel, conf.thumb_r_y, current.Gamepad.sThumbRY);
			}

#define BUTTON(bit, field) \
			if((state[i].Gamepad.wButtons ^ current.Gamepad.wButtons) & bit) { \
				PROCESSButton(device, conf.channel, conf.field, current.Gamepad.wButtons & bit); \
			}

			BUTTON(XINPUT_GAMEPAD_DPAD_UP, dpad_up)
			BUTTON(XINPUT_GAMEPAD_DPAD_DOWN, dpad_down)
			BUTTON(XINPUT_GAMEPAD_DPAD_LEFT, dpad_left)
			BUTTON(XINPUT_GAMEPAD_DPAD_RIGHT, dpad_right)
			BUTTON(XINPUT_GAMEPAD_START, start)
			BUTTON(XINPUT_GAMEPAD_BACK, back)
			BUTTON(XINPUT_GAMEPAD_LEFT_THUMB, left_thumb)
			BUTTON(XINPUT_GAMEPAD_RIGHT_THUMB, right_thumb)
			BUTTON(XINPUT_GAMEPAD_LEFT_SHOULDER, left_shoulder)
			BUTTON(XINPUT_GAMEPAD_RIGHT_SHOULDER, right_shoulder)
			BUTTON(XINPUT_GAMEPAD_A, a)
			BUTTON(XINPUT_GAMEPAD_B, b)
			BUTTON(XINPUT_GAMEPAD_X, x)
			BUTTON(XINPUT_GAMEPAD_Y, y)

			memcpy(&state[i], &current, sizeof(current));
		}
	}

	printf("Closing MIDI port\n");

	/* close device */
	midiOutReset(device);
	midiOutClose(device);

	return 0;
}
