#include "output-dialog.hpp"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QPushButton>
#include <QAbstractButton>
#include <QToolButton>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSizePolicy>
#include "obs-module.h"
#include <obs.h>
#include <QMessageBox>
#include "util/platform.h"
#include "stream-key-input.hpp"

// Reset output values, e.g. when user hits the back button
void OutputDialog::resetOutputs()
{
	outputName = QString("");
	outputServer = QString("");
	outputKey = QString("");
}

// For when we're done
void OutputDialog::acceptOutputs()
{
	accept();
}

// For validating the current values and then updating the confirm button state
void OutputDialog::validateOutputs(QPushButton *confirmButton)
{

	if (outputName.isEmpty() || otherNames.contains(outputName)) {
		confirmButton->setEnabled(false);
		return;
	}

	const QString method = tiktokSettings
		? QString::fromUtf8(obs_data_get_string(tiktokSettings, "tiktok_method"))
		: QString();
	const bool dynamicTikTok = verticalCanvasTarget && tiktokTarget &&
		!method.isEmpty() && method != "custom";
	confirmButton->setEnabled(dynamicTikTok || (!outputServer.isEmpty() && !outputKey.isEmpty()));
}

// Helper for generating info QLabels
QLabel *generateInfoLabel(std::string text)
{
	auto label = new QLabel;
	label->setTextFormat(Qt::RichText);
	label->setOpenExternalLinks(true);
	label->setText(QString::fromUtf8(obs_module_text(text.c_str())));
	label->setWordWrap(true);
	label->setAlignment(Qt::AlignTop);

	return label;
}

// Helper for generating form QLabels
QLabel *generateFormLabel(std::string text)
{
	auto label = new QLabel(QString::fromUtf8(obs_module_text(text.c_str())));
	label->setStyleSheet("font-weight: bold;");

	return label;
}

// Helper for generating output name field
QLineEdit *OutputDialog::generateOutputNameField(std::string text, QPushButton *confirmButton, bool edit)
{
	auto field = new QLineEdit;
	field->setText(QString::fromUtf8(obs_module_text(text.c_str())));
	field->setStyleSheet("padding: 4px 8px;");

	if (edit) { // edit mode, set field value from output value
		field->setText(outputName);
	}

	connect(field, &QLineEdit::textEdited, [this, field, confirmButton] {
		outputName = field->text();
		validateOutputs(confirmButton);
	});

	return field;
}

// Helper for generating output server field
QLineEdit *OutputDialog::generateOutputServerField(QPushButton *confirmButton, bool locked, bool edit)
{
	auto field = new QLineEdit;
	field->setStyleSheet("padding: 4px 8px;");
	field->setDisabled(locked);

	if (edit) { // edit mode, set field value from output value
		field->setText(outputServer);
	}

	if (!locked) {
		connect(field, &QLineEdit::textEdited, [this, field, confirmButton] {
			outputServer = field->text();
			validateOutputs(confirmButton);
		});
	}

	return field;
}

// Helper for generating QComboBoxes for server selection
QComboBox *OutputDialog::generateOutputServerCombo(std::string service, QPushButton *confirmButton, bool edit)
{
	auto combo = new QComboBox;
	combo->setMinimumHeight(30);
	combo->setStyleSheet("padding: 4px 8px;");

	if (service == "Twitch") {
		auto twitch_cache = obs_module_get_config_path(obs_get_module("rtmp-services"), "twitch_ingests.json");
		if (twitch_cache) {
			auto json = obs_data_create_from_json_file(twitch_cache);
			bfree(twitch_cache);
			combo->addItem(QString::fromUtf8("Default"), QString::fromUtf8("rtmp://live.twitch.tv/app"));
			auto ingests = obs_data_get_array(json, "ingests");
			obs_data_array_enum(
				ingests,
				[](obs_data_t *ingest_data, void *param) {
					auto c = (QComboBox *)param;
					auto url = QString::fromUtf8(obs_data_get_string(ingest_data, "url_template"));
					url.replace(QString::fromUtf8("/{stream_key}"), QString::fromUtf8(""));
					c->addItem(QString::fromUtf8(obs_data_get_string(ingest_data, "name")), url);
				},
				combo);
			obs_data_array_release(ingests);
			obs_data_release(json);
		}
	}

	auto rawOptions = getService(service);

	// turn raw options into actual selectable options
	if (rawOptions != nullptr) {
		auto servers = obs_data_get_array(rawOptions, "servers");
		auto totalServers = obs_data_array_count(servers);

		for (size_t idx = 0; idx < totalServers; idx++) {
			auto item = obs_data_array_item(servers, idx);
			combo->addItem(obs_data_get_string(item, "name"), obs_data_get_string(item, "url"));
		}
	}

	// If we're edit, look up the current value for outputServer and try to set the index
	if (edit) {
		auto selectionIndex = combo->findData(outputServer);
		//		blog(LOG_WARNING, "[Aitum Multistream] edit selection %i for value %s", selectionIndex, outputServer.toStdString().c_str());
		if (selectionIndex != -1) {
			combo->setCurrentIndex(selectionIndex);
		}
	}

	// Hook event up after
	connect(combo, &QComboBox::currentIndexChanged, [this, combo, confirmButton] {
		outputServer = combo->currentData().toString();
		validateOutputs(confirmButton);
	});

	// Set default value for server
	//	outputServer = combo->currentData().toString();

	return combo;
}

// Helper for generating output key field
QLineEdit *OutputDialog::generateOutputKeyField(QPushButton *confirmButton, bool edit)
{
	auto field = new StreamKeyInput;
	field->setStyleSheet("padding: 4px 8px;");

	if (edit) { // edit mode, set field value from output value
		field->setText(outputKey);
	}

	// Immediately hide
	field->setEchoMode(StreamKeyInput::EchoMode::Password);

	// On focus, show field
	connect(field, &StreamKeyInput::focusGained, [this, field] { field->setEchoMode(StreamKeyInput::EchoMode::Normal); });

	// On blur, hide field
	connect(field, &StreamKeyInput::focusLost, [this, field] { field->setEchoMode(StreamKeyInput::EchoMode::Password); });

	connect(field, &QLineEdit::textEdited, [this, field, confirmButton] {
		outputKey = field->text();
		validateOutputs(confirmButton);
	});

	return field;
}

// Helper for generating wizard button layout
QHBoxLayout *OutputDialog::generateWizardButtonLayout(QPushButton *confirmButton, QPushButton *serviceButton, bool edit)
{
	auto layout = new QHBoxLayout;
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	if (!edit && serviceButton != nullptr) { // Not edit mode and we have a service (back) button
		layout->addWidget(serviceButton, 1);
		layout->addStretch(1);

		layout->addWidget(confirmButton, 0, Qt::AlignRight);
	} else { // edit mode, push the button to the right
		layout->addWidget(confirmButton, 0, Qt::AlignRight);
	}

	return layout;
}

// Helper for generating QPushButtons w/ style
QPushButton *generateButton(QString text)
{
	auto button = new QPushButton;
	button->setText(text);
	button->setStyleSheet("padding: 4px 12px;");
	button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	return button;
}

// Helper for generating back button
QPushButton *OutputDialog::generateBackButton()
{
	auto button = generateButton(QString("< Back"));

	connect(button, &QPushButton::clicked, [this] {
		stackedWidget->setCurrentIndex(0);
		resetOutputs();
	});

	return button;
}

QToolButton *OutputDialog::selectionButton(std::string title, QIcon icon, int selectionStep)
{
	auto button = new QToolButton;

	button->setText(QString::fromUtf8(title));
	button->setIcon(icon);
	button->setIconSize(QSize(32, 32));
	button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	button->setStyleSheet(
		"min-width: 110px; max-width: 110px; min-height: 90px; max-height: 90px; padding-top: 16px; font-weight: bold;");

	connect(button, &QPushButton::clicked, [this, selectionStep] {
		if (verticalCanvasTarget)
			tiktokTarget = selectionStep == 7;
		stackedWidget->setCurrentIndex(selectionStep);
	});

	return button;
}

// Gets a service from the service array matched by name
obs_data_t *OutputDialog::getService(std::string serviceName)
{
	auto total = obs_data_array_count(servicesData);

	for (size_t idx = 0; idx < total; idx++) {
		auto item = obs_data_array_item(servicesData, idx);
		//		blog(LOG_WARNING, "[Aitum Multistream] %s", obs_data_get_string(item, "name"));

		if (serviceName == obs_data_get_string(item, "name")) {
			return item;
		}
	}

	return nullptr;
}

OutputDialog::OutputDialog(QDialog *parent, QStringList _otherNames, bool _verticalCanvasTarget)
	: QDialog(parent), otherNames(_otherNames), verticalCanvasTarget(_verticalCanvasTarget)
{
	// Load the services from rtmp-services plugin
	auto servicesPath = obs_module_get_config_path(obs_get_module("rtmp-services"), "services.json");
	auto absolutePath = os_get_abs_path_ptr(servicesPath);
	auto allData = obs_data_create_from_json_file(absolutePath);

	servicesData = obs_data_get_array(allData, "services");

	bfree(allData);
	bfree(servicesPath);
	bfree(absolutePath);

	// Blank info
	resetOutputs();
	if (verticalCanvasTarget) {
		tiktokSettings = obs_data_create();
	}

	//
	setModal(true);
	setContentsMargins(0, 0, 0, 0);
	setFixedSize(650, verticalCanvasTarget ? 480 : 400);

	stackedWidget = new QStackedWidget;

	stackedWidget->setStyleSheet("padding: 0px; margin: 0px;");

	// Service selection page
	stackedWidget->addWidget(WizardServicePage());

	// Pages for each service
	stackedWidget->addWidget(WizardInfoKick());
	stackedWidget->addWidget(WizardInfoYouTube());
	stackedWidget->addWidget(WizardInfoTwitter());
	stackedWidget->addWidget(WizardInfoUnknown());
	stackedWidget->addWidget(WizardInfoTwitch());
	stackedWidget->addWidget(WizardInfoTrovo());
	stackedWidget->addWidget(WizardInfoTikTok());
	stackedWidget->addWidget(WizardInfoFacebook());

	stackedWidget->setCurrentIndex(0);

	stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setWindowTitle(obs_module_text("NewOutputWindowTitle"));

	auto stackedLayout = new QVBoxLayout;
	stackedLayout->setContentsMargins(0, 0, 0, 0);
	stackedLayout->addWidget(stackedWidget, 1);

	setLayout(stackedLayout);
	show();
}

// Edit mode
OutputDialog::OutputDialog(QDialog *parent, QString name, QString server, QString key, QStringList _otherNames,
				   bool _verticalCanvasTarget, obs_data_t *existingSettings)
	: QDialog(parent),
	  otherNames(_otherNames),
	  verticalCanvasTarget(_verticalCanvasTarget)
{

	// Load the services from rtmp-services plugin
	auto module = obs_get_module("rtmp-services");
	auto servicesPath = module ? obs_module_get_config_path(module, "services.json") : nullptr;
	auto absolutePath = servicesPath ? os_get_abs_path_ptr(servicesPath) : nullptr;
	auto allData = absolutePath ? obs_data_create_from_json_file(absolutePath) : nullptr;

	servicesData = obs_data_get_array(allData, "services");

	obs_data_release(allData);
	bfree(servicesPath);
	bfree(absolutePath);

	// Blank info
	resetOutputs();

	// Set info from constructor
	outputName = name;
	outputServer = server;
	outputKey = key;
	twitchEnhancedBroadcasting = existingSettings && obs_data_get_bool(existingSettings, "twitch_enhanced_broadcasting");
	if (verticalCanvasTarget) {
		tiktokSettings = obs_data_create();
		if (existingSettings)
			obs_data_apply(tiktokSettings, existingSettings);
		tiktokTarget = obs_data_get_bool(tiktokSettings, "tiktok_target") ||
			obs_data_has_user_value(tiktokSettings, "tiktok_method") ||
			server.contains("tiktokcdn-", Qt::CaseInsensitive) ||
			server.contains("tiktok", Qt::CaseInsensitive) ||
			name.contains("tiktok", Qt::CaseInsensitive);
	}

	//
	setModal(true);
	setContentsMargins(0, 0, 0, 0);
	setFixedSize(650, 400);

	auto layout = new QVBoxLayout();

	// Add the appropriate page to the layout based upon the server URL.
	// A native TikTok target can use a placeholder RTMP URL, so identify it from
	// the settings marker rather than trying to infer its provider from the URL.
	if (verticalCanvasTarget && tiktokTarget) {
		layout->addWidget(WizardInfoTikTok(true));
	} else if (outputServer.contains(QString::fromUtf8("ingest.global-contribute.live-video.net")) ||
	    outputServer.contains(QString::fromUtf8(".contribute.live-video.net")) ||
	    outputServer.contains(QString::fromUtf8(".twitch.tv"))) { // twitch
		layout->addWidget(WizardInfoTwitch(true));
	} else if (outputServer.contains(QString::fromUtf8(".youtube.com"))) { // youtube
		layout->addWidget(WizardInfoYouTube(true));
	} else if (outputServer.contains(QString::fromUtf8("fa723fc1b171.global-contribute.live-video.net"))) { // kick
		layout->addWidget(WizardInfoKick(true));
	} else if (outputServer.contains(QString::fromUtf8(".tiktokcdn-"))) { // tiktok
		layout->addWidget(WizardInfoTikTok(true));
	} else if (outputServer.contains(QString::fromUtf8(".pscp.tv"))) { // twitter
		layout->addWidget(WizardInfoTwitter(true));
	} else if (outputServer.contains(QString::fromUtf8("livepush.trovo.live"))) { // trovo
		layout->addWidget(WizardInfoTrovo(true));
	} else if (outputServer.contains(QString::fromUtf8(".fbcdn.net")) ||
		   outputServer.contains(QString::fromUtf8(".facebook.com"))) { // facebook
		layout->addWidget(WizardInfoFacebook(true));
	} else { // unknown
		layout->addWidget(WizardInfoUnknown(true));
	}

	setLayout(layout);

	setWindowTitle(obs_module_text("EditOutputWindowTitle"));

	show();
}

OutputDialog::~OutputDialog()
{
	obs_data_array_release(servicesData);
	if (tiktokSettings)
		obs_data_release(tiktokSettings);
}

QWidget *OutputDialog::WizardServicePage()
{
	auto page = new QWidget(this);
	auto pageLayout = new QVBoxLayout;
	pageLayout->setAlignment(Qt::AlignTop);

	auto description = new QLabel(QString::fromUtf8(obs_module_text("NewOutputSelectService")));
	description->setWordWrap(true);
	description->setStyleSheet("margin-bottom: 20px;");
	pageLayout->addWidget(description);

	// layout for service selection
	auto selectionLayout = new QVBoxLayout;
	selectionLayout->setAlignment(Qt::AlignCenter);

	auto spacerTest = new QSpacerItem(20, 45, QSizePolicy::Fixed, QSizePolicy::Fixed);

	pageLayout->addSpacerItem(spacerTest);

	// row 1
	auto rowOne = new QHBoxLayout;

	rowOne->addWidget(selectionButton("Twitch", platformIconTwitch, 5));
	rowOne->addWidget(selectionButton("YouTube", platformIconYouTube, 2));
	rowOne->addWidget(selectionButton("TikTok", platformIconTikTok, 7));
	rowOne->addWidget(selectionButton("Facebook", platformIconFacebook, 8));

	selectionLayout->addLayout(rowOne);

	// row 2
	auto rowTwo = new QHBoxLayout;

	rowTwo->addWidget(selectionButton("Trovo", platformIconTrovo, 6));
	rowTwo->addWidget(selectionButton("X (Twitter)", platformIconTwitter, 3));
	rowTwo->addWidget(selectionButton("Kick", platformIconKick, 1));
	rowTwo->addWidget(selectionButton(obs_module_text("OtherService"), platformIconUnknown, 4));

	selectionLayout->addLayout(rowTwo);

	//
	pageLayout->addLayout(selectionLayout);
	page->setLayout(pageLayout);

	return page;
}

QWidget *OutputDialog::WizardInfoKick(bool edit)
{
	auto page = new QWidget(this);
	page->setStyleSheet("padding: 0px; margin: 0px;");

	// Layout
	auto pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(12);

	// Heading
	auto title = new QLabel(QString::fromUtf8(obs_module_text(edit ? "KickServiceInfoEdit" : "KickServiceInfo")));
	title->setWordWrap(true);
	title->setTextFormat(Qt::RichText);
	pageLayout->addWidget(title);

	// Content
	auto contentLayout = new QVBoxLayout;

	// Confirm button - initialised here so we can set state in form input connects
	auto confirmButton = generateButton(QString(obs_module_text(edit ? "SaveOutput" : "CreateOutput")));

	// Form
	auto formLayout = new QFormLayout;
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	formLayout->setSpacing(12);

	// Output name
	auto outputNameField = generateOutputNameField("KickOutput", confirmButton, edit);
	formLayout->addRow(generateFormLabel("OutputName"), outputNameField);

	// Server field
	auto serverSelection = generateOutputServerField(confirmButton, true, edit);
	serverSelection->setText("rtmps://fa723fc1b171.global-contribute.live-video.net");
	formLayout->addRow(generateFormLabel("KickServer"), serverSelection);

	// Server info
	formLayout->addWidget(generateInfoLabel("KickServerInfo"));

	// Server key
	auto outputKeyField = generateOutputKeyField(confirmButton, edit);
	formLayout->addRow(generateFormLabel("KickStreamKey"), outputKeyField);

	// Server key info
	formLayout->addWidget(generateInfoLabel("KickStreamKeyInfo"));

	contentLayout->addLayout(formLayout);

	// spacing
	auto spacer = new QSpacerItem(1, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	contentLayout->addSpacerItem(spacer);

	pageLayout->addLayout(contentLayout);

	// back button
	auto serviceButton = edit ? nullptr : generateBackButton();

	// Controls layout
	auto controlsLayout = generateWizardButtonLayout(confirmButton, serviceButton, edit);

	// confirm button (initialised above so we can set state)
	connect(confirmButton, &QPushButton::clicked, [this] { acceptOutputs(); });

	// Hook it all together
	pageLayout->addLayout(controlsLayout, 1);
	page->setLayout(pageLayout);

	// Defaults for when we're changed to
	if (!edit) {
		connect(stackedWidget, &QStackedWidget::currentChanged,
			[this, outputNameField, serverSelection, outputKeyField, confirmButton] {
				if (stackedWidget->currentIndex() == 1) {
					outputName = outputNameField->text();
					outputServer = serverSelection->text();
					outputKey = outputKeyField->text();
					validateOutputs(confirmButton);
				}
			});
	}

	return page;
}

QWidget *OutputDialog::WizardInfoYouTube(bool edit)
{
	auto page = new QWidget(this);
	page->setStyleSheet("padding: 0px; margin: 0px;");

	// Layout
	auto pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(12);

	// Heading
	auto title = new QLabel(QString::fromUtf8(obs_module_text("YouTubeServiceInfo")));
	pageLayout->addWidget(title);

	// Content
	auto contentLayout = new QVBoxLayout;

	// Confirm button - initialised here so we can set state in form input connects
	auto confirmButton = generateButton(QString(obs_module_text(edit ? "SaveOutput" : "CreateOutput")));

	// Form
	auto formLayout = new QFormLayout;
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	formLayout->setSpacing(12);

	// Output name
	auto outputNameField = generateOutputNameField("YouTubeOutput", confirmButton, edit);
	formLayout->addRow(generateFormLabel("OutputName"), outputNameField);

	// Server selection
	auto serverSelection = generateOutputServerCombo("YouTube - RTMPS", confirmButton, edit);
	formLayout->addRow(generateFormLabel("YouTubeServer"), serverSelection);

	// Server info
	formLayout->addWidget(generateInfoLabel("YouTubeServerInfo"));

	// Server key
	auto outputKeyField = generateOutputKeyField(confirmButton, edit);
	formLayout->addRow(generateFormLabel("YouTubeStreamKey"), outputKeyField);

	// Server key info
	formLayout->addWidget(generateInfoLabel("YouTubeStreamKeyInfo"));

	contentLayout->addLayout(formLayout);

	// spacing
	auto spacer = new QSpacerItem(1, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	contentLayout->addSpacerItem(spacer);

	pageLayout->addLayout(contentLayout);

	// back button
	auto serviceButton = edit ? nullptr : generateBackButton();

	// Controls layout
	auto controlsLayout = generateWizardButtonLayout(confirmButton, serviceButton, edit);

	// confirm button (initialised above so we can set state)
	connect(confirmButton, &QPushButton::clicked, [this] { acceptOutputs(); });

	// Hook it all together
	pageLayout->addLayout(controlsLayout, 1);
	page->setLayout(pageLayout);

	// Defaults for when we're changed to
	if (!edit) {
		connect(stackedWidget, &QStackedWidget::currentChanged,
			[this, outputNameField, serverSelection, outputKeyField, confirmButton] {
				if (stackedWidget->currentIndex() == 2) {
					outputName = outputNameField->text();
					outputServer = serverSelection->currentData().toString();
					outputKey = outputKeyField->text();
					validateOutputs(confirmButton);
				}
			});
	}

	return page;
}

QWidget *OutputDialog::WizardInfoTwitter(bool edit)
{
	auto page = new QWidget(this);
	page->setStyleSheet("padding: 0px; margin: 0px;");

	// Layout
	auto pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(12);

	// Heading
	auto title = new QLabel(QString::fromUtf8(obs_module_text("TwitterServiceInfo")));
	pageLayout->addWidget(title);

	// Content
	auto contentLayout = new QVBoxLayout;

	// Confirm button - initialised here so we can set state in form input connects
	auto confirmButton = generateButton(QString(obs_module_text(edit ? "SaveOutput" : "CreateOutput")));

	// Form
	auto formLayout = new QFormLayout;
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	formLayout->setSpacing(12);

	// Output name
	auto outputNameField = generateOutputNameField("TwitterOutput", confirmButton, edit);
	formLayout->addRow(generateFormLabel("OutputName"), outputNameField);

	// Server selection
	auto serverSelection = generateOutputServerCombo("Twitter", confirmButton, edit);
	formLayout->addRow(generateFormLabel("TwitterServer"), serverSelection);

	// Server info
	formLayout->addWidget(generateInfoLabel("TwitterServerInfo"));

	// Server key
	auto outputKeyField = generateOutputKeyField(confirmButton, edit);
	formLayout->addRow(generateFormLabel("TwitterStreamKey"), outputKeyField);

	// Server key info
	formLayout->addWidget(generateInfoLabel("TwitterStreamKeyInfo"));

	contentLayout->addLayout(formLayout);

	// spacing
	auto spacer = new QSpacerItem(1, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	contentLayout->addSpacerItem(spacer);

	pageLayout->addLayout(contentLayout);

	// back button
	auto serviceButton = edit ? nullptr : generateBackButton();

	// Controls layout
	auto controlsLayout = generateWizardButtonLayout(confirmButton, serviceButton, edit);

	// confirm button (initialised above so we can set state)
	connect(confirmButton, &QPushButton::clicked, [this] { acceptOutputs(); });

	// Hook it all together
	pageLayout->addLayout(controlsLayout, 1);
	page->setLayout(pageLayout);

	// Defaults for when we're changed to
	if (!edit) {
		connect(stackedWidget, &QStackedWidget::currentChanged,
			[this, outputNameField, serverSelection, outputKeyField, confirmButton] {
				if (stackedWidget->currentIndex() == 3) {
					outputName = outputNameField->text();
					outputServer = serverSelection->currentData().toString();
					outputKey = outputKeyField->text();
					validateOutputs(confirmButton);
				}
			});
	}

	return page;
}

QWidget *OutputDialog::WizardInfoUnknown(bool edit)
{
	auto page = new QWidget(this);
	page->setStyleSheet("padding: 0px; margin: 0px;");

	// Layout
	auto pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(12);

	// Heading
	auto title = new QLabel(QString::fromUtf8(obs_module_text("CustomServiceInfo")));
	title->setWordWrap(true);
	title->setTextFormat(Qt::RichText);
	pageLayout->addWidget(title);

	// Content
	auto contentLayout = new QVBoxLayout;

	// Confirm button - initialised here so we can set state in form input connects
	auto confirmButton = generateButton(QString(obs_module_text(edit ? "SaveOutput" : "CreateOutput")));

	// Form
	auto formLayout = new QFormLayout;
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	formLayout->setSpacing(12);

	// Output name
	auto outputNameField = generateOutputNameField("CustomOutput", confirmButton, edit);
	formLayout->addRow(generateFormLabel("OutputName"), outputNameField);

	// Server field
	auto serverSelection = generateOutputServerField(confirmButton, false, edit);
	formLayout->addRow(generateFormLabel("CustomServer"), serverSelection);

	// Server info
	formLayout->addWidget(generateInfoLabel("CustomServerInfo"));

	// Server key
	auto outputKeyField = generateOutputKeyField(confirmButton, edit);
	formLayout->addRow(generateFormLabel("CustomStreamKey"), outputKeyField);

	// Server key info
	formLayout->addWidget(generateInfoLabel("CustomStreamKeyInfo"));

	contentLayout->addLayout(formLayout);

	// spacing
	auto spacer = new QSpacerItem(1, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	contentLayout->addSpacerItem(spacer);

	pageLayout->addLayout(contentLayout);

	// back button
	auto serviceButton = edit ? nullptr : generateBackButton();

	// Controls layout
	auto controlsLayout = generateWizardButtonLayout(confirmButton, serviceButton, edit);

	// confirm button (initialised above so we can set state)
	connect(confirmButton, &QPushButton::clicked, [this] { acceptOutputs(); });

	// Hook it all together
	pageLayout->addLayout(controlsLayout, 1);
	page->setLayout(pageLayout);

	// Defaults for when we're changed to
	if (!edit) {
		connect(stackedWidget, &QStackedWidget::currentChanged,
			[this, outputNameField, serverSelection, outputKeyField, confirmButton] {
				if (stackedWidget->currentIndex() == 4) {
					outputName = outputNameField->text();
					outputServer = serverSelection->text();
					outputKey = outputKeyField->text();
					validateOutputs(confirmButton);
				}
			});
	}

	return page;
}

QWidget *OutputDialog::WizardInfoTwitch(bool edit)
{
	auto page = new QWidget(this);
	page->setStyleSheet("padding: 0px; margin: 0px;");

	// Layout
	auto pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(12);

	// Heading
	auto title = new QLabel(QString::fromUtf8(obs_module_text("TwitchServiceInfo")));
	pageLayout->addWidget(title);

	// Content
	auto contentLayout = new QVBoxLayout;

	// Confirm button - initialised here so we can set state in form input connects
	auto confirmButton = generateButton(QString(obs_module_text(edit ? "SaveOutput" : "CreateOutput")));

	// Form
	auto formLayout = new QFormLayout;
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	formLayout->setSpacing(12);

	// Output name
	auto outputNameField = generateOutputNameField("TwitchOutput", confirmButton, edit);
	formLayout->addRow(generateFormLabel("OutputName"), outputNameField);

	// Server selection
	auto serverSelection = generateOutputServerCombo("Twitch", confirmButton, edit);
	formLayout->addRow(generateFormLabel("TwitchServer"), serverSelection);

	// Server info
	formLayout->addWidget(generateInfoLabel("TwitchServerInfo"));

	// Server key
	auto outputKeyField = generateOutputKeyField(confirmButton, edit);
	formLayout->addRow(generateFormLabel("TwitchStreamKey"), outputKeyField);

	// Server key info
	formLayout->addWidget(generateInfoLabel("TwitchStreamKeyInfo"));

	// Enhanced Broadcasting is an output-level option, independent of OBS's
	// built-in stream output and any other destination.
	auto enhancedBroadcasting = new QCheckBox(QString::fromUtf8(obs_module_text("TwitchEnhancedBroadcasting")));
	enhancedBroadcasting->setChecked(twitchEnhancedBroadcasting);
	enhancedBroadcasting->setToolTip(QString::fromUtf8(obs_module_text("TwitchEnhancedBroadcastingInfo")));
	connect(enhancedBroadcasting, &QCheckBox::toggled, [this](bool enabled) {
		twitchEnhancedBroadcasting = enabled;
	});
	formLayout->addRow(QString(), enhancedBroadcasting);

	contentLayout->addLayout(formLayout);

	// spacing
	auto spacer = new QSpacerItem(1, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	contentLayout->addSpacerItem(spacer);

	pageLayout->addLayout(contentLayout);

	// back button
	auto serviceButton = edit ? nullptr : generateBackButton();

	// Controls layout
	auto controlsLayout = generateWizardButtonLayout(confirmButton, serviceButton, edit);

	// confirm button (initialised above so we can set state)
	connect(confirmButton, &QPushButton::clicked, [this] { acceptOutputs(); });

	// Hook it all together
	pageLayout->addLayout(controlsLayout, 1);
	page->setLayout(pageLayout);

	// Defaults for when we're changed to
	if (!edit) {
		connect(stackedWidget, &QStackedWidget::currentChanged,
			[this, outputNameField, serverSelection, outputKeyField, confirmButton] {
				if (stackedWidget->currentIndex() == 5) {
					blog(LOG_WARNING, "[Aitum Multistream] default outputname %s ",
					     outputNameField->text().toUtf8().constData());
					outputName = outputNameField->text();
					outputServer = serverSelection->currentData().toString();
					outputKey = outputKeyField->text();
					validateOutputs(confirmButton);
				}
			});
	}

	return page;
}

QWidget *OutputDialog::WizardInfoTrovo(bool edit)
{
	auto page = new QWidget(this);
	page->setStyleSheet("padding: 0px; margin: 0px;");

	// Layout
	auto pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(12);

	// Heading
	auto title = new QLabel(QString::fromUtf8(obs_module_text("TrovoServiceInfo")));
	title->setWordWrap(true);
	title->setTextFormat(Qt::RichText);
	pageLayout->addWidget(title);

	// Content
	auto contentLayout = new QVBoxLayout;

	// Confirm button - initialised here so we can set state in form input connects
	auto confirmButton = generateButton(QString(obs_module_text(edit ? "SaveOutput" : "CreateOutput")));

	// Form
	auto formLayout = new QFormLayout;
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	formLayout->setSpacing(12);

	// Output name
	auto outputNameField = generateOutputNameField("TrovoOutput", confirmButton, edit);
	formLayout->addRow(generateFormLabel("OutputName"), outputNameField);

	// Server field
	auto serverSelection = generateOutputServerField(confirmButton, true, edit);
	serverSelection->setText("rtmp://livepush.trovo.live/live/");

	formLayout->addRow(generateFormLabel("TrovoServer"), serverSelection);

	// Server info
	formLayout->addWidget(generateInfoLabel("TrovoServerInfo"));

	// Server key
	auto outputKeyField = generateOutputKeyField(confirmButton, edit);
	formLayout->addRow(generateFormLabel("TrovoStreamKey"), outputKeyField);

	// Server key info
	formLayout->addWidget(generateInfoLabel("TrovoStreamKeyInfo"));

	contentLayout->addLayout(formLayout);

	// spacing
	auto spacer = new QSpacerItem(1, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	contentLayout->addSpacerItem(spacer);

	pageLayout->addLayout(contentLayout);

	// back button
	auto serviceButton = edit ? nullptr : generateBackButton();

	// Controls layout
	auto controlsLayout = generateWizardButtonLayout(confirmButton, serviceButton, edit);

	// confirm button (initialised above so we can set state)
	connect(confirmButton, &QPushButton::clicked, [this] { acceptOutputs(); });

	// Hook it all together
	pageLayout->addLayout(controlsLayout, 1);
	page->setLayout(pageLayout);

	// Defaults for when we're changed to
	if (!edit) {
		connect(stackedWidget, &QStackedWidget::currentChanged,
			[this, outputNameField, serverSelection, outputKeyField, confirmButton] {
				if (stackedWidget->currentIndex() == 6) {
					outputName = outputNameField->text();
					outputServer = serverSelection->text();
					outputKey = outputKeyField->text();
					validateOutputs(confirmButton);
				}
			});
	}

	return page;
}

bool OutputDialog::ConfigureVerticalTikTok(QWidget *parent)
{
	if (!verticalCanvasTarget || !tiktokSettings)
		return false;

	auto *handler = obs_get_proc_handler();
	if (!handler) {
		QMessageBox::warning(parent, "TikTok", "Aitum Vertical Canvas is not available.");
		return false;
	}

	calldata_t cd;
	calldata_init(&cd);
	const QByteArray outputNameBytes = outputName.toUtf8();
	calldata_set_ptr(&cd, "parent", parent);
	calldata_set_ptr(&cd, "settings", tiktokSettings);
	calldata_set_string(&cd, "name", outputNameBytes.constData());
	const bool called = proc_handler_call(handler, "aitum_vertical_configure_tiktok_output", &cd);
	const bool accepted = called && calldata_bool(&cd, "accepted");
	calldata_free(&cd);

	if (!called) {
		QMessageBox::warning(parent, "TikTok",
			"Aitum Vertical Canvas is outdated or does not expose the TikTok configuration dialog. Update Vertical Canvas first.");
	}
	return accepted;
}

QWidget *OutputDialog::WizardInfoTikTok(bool edit)
{
	if (verticalCanvasTarget)
		return WizardInfoVerticalTikTok(edit);

	auto page = new QWidget(this);
	page->setStyleSheet("padding: 0px; margin: 0px;");

	// Layout
	auto pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(12);

	// Heading
	auto title = new QLabel(QString::fromUtf8(obs_module_text("TikTokServiceInfo")));
	title->setWordWrap(true);
	title->setTextFormat(Qt::RichText);
	pageLayout->addWidget(title);

	// Content
	auto contentLayout = new QVBoxLayout;

	// Confirm button - initialised here so we can set state in form input connects
	auto confirmButton = generateButton(QString(obs_module_text(edit ? "SaveOutput" : "CreateOutput")));

	// Form
	auto formLayout = new QFormLayout;
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	formLayout->setSpacing(12);

	// Output name
	auto outputNameField = generateOutputNameField("TikTokOutput", confirmButton, edit);
	formLayout->addRow(generateFormLabel("OutputName"), outputNameField);

	// Server field
	auto serverSelection = generateOutputServerField(confirmButton, false, edit);
	formLayout->addRow(generateFormLabel("TikTokServer"), serverSelection);

	// Server info
	formLayout->addWidget(generateInfoLabel("TikTokServerInfo"));

	// Server key
	auto outputKeyField = generateOutputKeyField(confirmButton, edit);
	formLayout->addRow(generateFormLabel("TikTokStreamKey"), outputKeyField);

	// Server key info
	formLayout->addWidget(generateInfoLabel("TikTokStreamKeyInfo"));

	contentLayout->addLayout(formLayout);

	// spacing
	auto spacer = new QSpacerItem(1, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	contentLayout->addSpacerItem(spacer);

	pageLayout->addLayout(contentLayout);

	// back button
	auto serviceButton = edit ? nullptr : generateBackButton();

	// Controls layout
	auto controlsLayout = generateWizardButtonLayout(confirmButton, serviceButton, edit);

	// confirm button (initialised above so we can set state)
	connect(confirmButton, &QPushButton::clicked, [this] { acceptOutputs(); });

	// Hook it all together
	pageLayout->addLayout(controlsLayout, 1);
	page->setLayout(pageLayout);

	// Defaults for when we're changed to
	if (!edit) {
		connect(stackedWidget, &QStackedWidget::currentChanged,
			[this, outputNameField, serverSelection, outputKeyField, confirmButton] {
				if (stackedWidget->currentIndex() == 7) {
					outputName = outputNameField->text();
					outputServer = serverSelection->text();
					outputKey = outputKeyField->text();
					validateOutputs(confirmButton);
				}
			});
	}

	return page;
}

QWidget *OutputDialog::WizardInfoVerticalTikTok(bool edit)
{
	auto page = new QWidget(this);
	page->setStyleSheet("padding: 0px; margin: 0px;");

	auto pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(12);
	auto title = new QLabel(edit
		? "Edit this TikTok destination for Vertical Canvas. Choose how it connects below."
		: "Add a TikTok destination for Vertical Canvas. Choose TikTok LIVE Studio Desktop or Mobile, Streamlabs Desktop, or Custom RTMP.");
	title->setWordWrap(true);
	pageLayout->addWidget(title);

	auto formLayout = new QFormLayout;
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	formLayout->setSpacing(12);

	auto confirmButton = generateButton(QString(obs_module_text(edit ? "SaveOutput" : "CreateOutput")));
	auto outputNameField = generateOutputNameField("TikTokOutput", confirmButton, edit);
	formLayout->addRow(generateFormLabel("OutputName"), outputNameField);

	auto configureButton = generateButton("Configure TikTok connection…");
	configureButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	auto methodSummary = new QLabel;
	methodSummary->setWordWrap(true);
	auto connectionInfo = new QLabel;
	connectionInfo->setWordWrap(true);
	formLayout->addRow(QString(), configureButton);
	formLayout->addRow(QString(), methodSummary);
	formLayout->addRow(QString(), connectionInfo);

	auto serverSelection = generateOutputServerField(confirmButton, false, edit);
	auto serverRow = new QWidget(page);
	auto serverLayout = new QVBoxLayout(serverRow);
	serverLayout->setContentsMargins(0, 0, 0, 0);
	serverLayout->addWidget(serverSelection);
	formLayout->addRow(generateFormLabel("TikTokServer"), serverRow);
	auto serverInfo = generateInfoLabel("TikTokServerInfo");
	formLayout->addRow(QString(), serverInfo);

	auto outputKeyField = generateOutputKeyField(confirmButton, edit);
	auto keyRow = new QWidget(page);
	auto keyRowLayout = new QVBoxLayout(keyRow);
	keyRowLayout->setContentsMargins(0, 0, 0, 0);
	keyRowLayout->addWidget(outputKeyField);
	formLayout->addRow(generateFormLabel("TikTokStreamKey"), keyRow);
	auto keyInfo = generateInfoLabel("TikTokStreamKeyInfo");
	formLayout->addRow(QString(), keyInfo);

	auto methodName = [this] {
		const QString method = tiktokSettings ? QString::fromUtf8(obs_data_get_string(tiktokSettings, "tiktok_method")) : QString();
		if (method == "live-studio") return QString("TikTok LIVE Studio — Desktop");
		if (method == "live-studio-mobile") return QString("TikTok LIVE Studio — Mobile");
		if (method == "streamlabs") return QString("Streamlabs Desktop");
		return QString("Custom RTMP / stream key");
	};
	auto updateMethod = [this, methodSummary, connectionInfo, serverRow, serverInfo, keyRow, keyInfo, serverSelection, outputKeyField, confirmButton, methodName] {
		const QString method = tiktokSettings ? QString::fromUtf8(obs_data_get_string(tiktokSettings, "tiktok_method")) : QString();
		const bool custom = method.isEmpty() || method == "custom";
		serverRow->setVisible(custom);
		serverInfo->setVisible(custom);
		keyRow->setVisible(custom);
		keyInfo->setVisible(custom);
		if (!custom) {
			outputServer = "rtmp://placeholder.invalid/live";
			outputKey = "pending";
			serverSelection->setText(outputServer);
			outputKeyField->setText(outputKey);
		} else {
			outputServer = serverSelection->text();
			outputKey = outputKeyField->text();
		}
		const bool configured = tiktokSettings && obs_data_has_user_value(tiktokSettings, "tiktok_method");
		methodSummary->setText(configured ? QString("Connection: %1").arg(methodName()) :
			QString("Choose a connection method. Custom RTMP can be entered below."));
		connectionInfo->setText(custom
			? QString("For Custom RTMP, enter the server and stream key supplied by your provider.")
			: QString("The provider will supply a fresh RTMP server and stream key when this Vertical Canvas output starts. You do not need to paste them here."));
		validateOutputs(confirmButton);
	};

	connect(configureButton, &QPushButton::clicked, this,
		 [this, outputNameField, serverSelection, outputKeyField, updateMethod] {
			outputName = outputNameField->text();
			const QString oldMethod = tiktokSettings
				? QString::fromUtf8(obs_data_get_string(tiktokSettings, "tiktok_method")) : QString();
			if (ConfigureVerticalTikTok(this)) {
				const QString newMethod = tiktokSettings
					? QString::fromUtf8(obs_data_get_string(tiktokSettings, "tiktok_method")) : QString();
				if ((newMethod.isEmpty() || newMethod == "custom") &&
					!oldMethod.isEmpty() && oldMethod != "custom") {
					serverSelection->clear();
					outputKeyField->clear();
				}
				updateMethod();
			}
		});

	connect(confirmButton, &QPushButton::clicked, [this, outputNameField, serverSelection, outputKeyField] {
		outputName = outputNameField->text();
		const QString method = tiktokSettings ? QString::fromUtf8(obs_data_get_string(tiktokSettings, "tiktok_method")) : QString();
		if (method.isEmpty() || method == "custom") {
			outputServer = serverSelection->text();
			outputKey = outputKeyField->text();
		} else {
			outputServer = "rtmp://placeholder.invalid/live";
			outputKey = "pending";
		}
		acceptOutputs();
	});

	pageLayout->addLayout(formLayout);
	pageLayout->addStretch(1);

	auto serviceButton = edit ? nullptr : generateBackButton();
	auto controlsLayout = generateWizardButtonLayout(confirmButton, serviceButton, edit);
	pageLayout->addLayout(controlsLayout);
	page->setLayout(pageLayout);

	if (edit) {
		outputNameField->setText(outputName);
	} else {
		connect(stackedWidget, &QStackedWidget::currentChanged, this,
			[this, outputNameField, serverSelection, outputKeyField, updateMethod](int index) {
				if (index == 7) {
					outputName = outputNameField->text();
					outputServer = serverSelection->text();
					outputKey = outputKeyField->text();
					updateMethod();
				}
			});
	}
	updateMethod();
	return page;
}

QWidget *OutputDialog::WizardInfoFacebook(bool edit)
{
	auto page = new QWidget(this);
	page->setStyleSheet("padding: 0px; margin: 0px;");

	// Layout
	auto pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(12);

	// Heading
	auto title = new QLabel(QString::fromUtf8(obs_module_text("FacebookServiceInfo")));
	title->setWordWrap(true);
	title->setTextFormat(Qt::RichText);
	pageLayout->addWidget(title);

	// Content
	auto contentLayout = new QVBoxLayout;

	// Confirm button - initialised here so we can set state in form input connects
	auto confirmButton = generateButton(QString(obs_module_text(edit ? "SaveOutput" : "CreateOutput")));

	// Form
	auto formLayout = new QFormLayout;
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	formLayout->setSpacing(12);

	// Output name
	auto outputNameField = generateOutputNameField("FacebookOutput", confirmButton, edit);
	formLayout->addRow(generateFormLabel("OutputName"), outputNameField);

	// Server field
	auto serverSelection = generateOutputServerField(confirmButton, false, edit);
	if (!edit)
		serverSelection->setText("rtmps://rtmp-api.facebook.com:443/rtmp/");

	formLayout->addRow(generateFormLabel("FacebookServer"), serverSelection);

	// Server info
	formLayout->addWidget(generateInfoLabel("FacebookServerInfo"));

	// Server key
	auto outputKeyField = generateOutputKeyField(confirmButton, edit);
	formLayout->addRow(generateFormLabel("FacebookStreamKey"), outputKeyField);

	// Server key info
	formLayout->addWidget(generateInfoLabel("FacebookStreamKeyInfo"));

	contentLayout->addLayout(formLayout);

	// spacing
	auto spacer = new QSpacerItem(1, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
	contentLayout->addSpacerItem(spacer);

	pageLayout->addLayout(contentLayout);

	// back button
	auto serviceButton = edit ? nullptr : generateBackButton();

	// Controls layout
	auto controlsLayout = generateWizardButtonLayout(confirmButton, serviceButton, edit);

	// confirm button (initialised above so we can set state)
	connect(confirmButton, &QPushButton::clicked, [this] { acceptOutputs(); });

	// Hook it all together
	pageLayout->addLayout(controlsLayout, 1);
	page->setLayout(pageLayout);

	// Defaults for when we're changed to
	if (!edit) {
		connect(stackedWidget, &QStackedWidget::currentChanged,
			[this, outputNameField, serverSelection, outputKeyField, confirmButton] {
				if (stackedWidget->currentIndex() == 8) {
					outputName = outputNameField->text();
					outputServer = serverSelection->text();
					outputKey = outputKeyField->text();
					validateOutputs(confirmButton);
				}
			});
	}

	return page;
}
