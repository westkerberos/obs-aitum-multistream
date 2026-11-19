#pragma once

#include <QPushButton>
#include <QComboBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QGroupBox>
#include <QIcon>
#include <QString>
#include <QToolButton>
#include "obs.h"

/*
 * Special Aitum video source: reuse the OBS-owned stream encoder that exists
 * alongside Twitch Enhanced Broadcasting's multitrack output.
 */
#define WEST_OBS_NATIVE_STREAM_ENCODER_OPTION "__west_obs_native_stream_encoder__"

inline obs_encoder_t *WestGetOBSNativeStreamEncoder()
{
	obs_encoder_t *encoder = obs_get_encoder_by_name("advanced_video_stream");
	if (!encoder)
		encoder = obs_get_encoder_by_name("simple_video_stream");
	return encoder;
}

class ConfigUtils {
public:
	// UI generation
	static QPushButton *generateButton(QString buttonText);
	static QGroupBox *generateSettingsGroupBox(QString headingText);
	static QToolButton *generateMenuButton(QString title, QIcon icon);
	
	static QIcon generateEmojiQIcon(QString emoji);

	static void updateButtonStyles(QPushButton *defaultButton, QPushButton *customButton, int activeIndex);

	static QIcon getPlatformIconFromEndpoint(QString endpoint);
};
