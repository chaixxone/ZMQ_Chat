#include <chat_ui.hpp>
#include <qt_helper_window.hpp>
#include <chat_text_frame.hpp>
#include <chat_text_line.hpp>
#include <popup_signal_emitting_q_combo_box.hpp>
#include <chat_invite.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace UI;

ChatUI::ChatUI(std::shared_ptr<Client> client, std::shared_ptr<QtMessageObserver> observer, QWidget* parent) :
	QMainWindow(parent), 
	_pages(new QStackedWidget(this)), 
	_registerPage(new QWidget),
	_loginPage(new QWidget), 
	_mainPage(new QWidget),
	_noticeBox(new NoticeBox("Notices")),
	_client(client),
	_messageObserver(observer)
{	
	_pages->addWidget(_loginPage);
	_pages->addWidget(_mainPage);
	_pages->addWidget(_registerPage);
	// TODO: remove current index setting
	_pages->setCurrentIndex(0);
	setCentralWidget(_pages);

	constexpr int lineEditsMaxWidth = 300;

	// register page
	auto registerLoginLineEdit = new QLineEdit;
	registerLoginLineEdit->setMaximumWidth(lineEditsMaxWidth);
	registerLoginLineEdit->setPlaceholderText("Enter login");	

	auto registerPasswordLineEdit = new QLineEdit;
	registerPasswordLineEdit->setMaximumWidth(lineEditsMaxWidth);
	registerPasswordLineEdit->setPlaceholderText("Enter password");
	registerPasswordLineEdit->setEchoMode(QLineEdit::EchoMode::Password);

	auto registerPasswordRepeatLineEdit = new QLineEdit;
	registerPasswordRepeatLineEdit->setMaximumWidth(lineEditsMaxWidth);
	registerPasswordRepeatLineEdit->setPlaceholderText("Repeat password");
	registerPasswordRepeatLineEdit->setEchoMode(QLineEdit::EchoMode::Password);

	auto vRegisterLayout = new QVBoxLayout;
	vRegisterLayout->addWidget(registerLoginLineEdit, 0);
	vRegisterLayout->addWidget(registerPasswordLineEdit, 0);
	vRegisterLayout->addWidget(registerPasswordRepeatLineEdit, 0);
	vRegisterLayout->setAlignment(Qt::AlignCenter);
	_registerPage->setLayout(vRegisterLayout);

	auto parseDataFromRegisterInput = [this, registerLoginLineEdit, registerPasswordLineEdit, registerPasswordRepeatLineEdit]() {
		QString login = registerLoginLineEdit->text().trimmed();
		QString password = registerPasswordLineEdit->text().trimmed();
		QString passwordRepeat = registerPasswordRepeatLineEdit->text().trimmed();

		if (!login.isEmpty() && !password.isEmpty() && !passwordRepeat.isEmpty())
		{
			_client->RequestRegister(login.toStdString(), password.toStdString(), passwordRepeat.toStdString());
		}
	};

	connect(registerLoginLineEdit, &QLineEdit::returnPressed, this, parseDataFromRegisterInput);
	connect(registerPasswordLineEdit, &QLineEdit::returnPressed, this, parseDataFromRegisterInput);
	connect(registerPasswordRepeatLineEdit, &QLineEdit::returnPressed, this, parseDataFromRegisterInput);

	connect(_messageObserver.get(), &QtMessageObserver::Register, this, [this](const MessageView& message) {
		json registerStatus = json::parse(message.Content);
		bool isRegistered = registerStatus["is_registered"].get<bool>();

		// TODO take login and password and paste them into login page inputs

		int nextPageIndex = isRegistered ? 0 : 2; // 2 - register page | 0 - login page

		_pages->setCurrentIndex(nextPageIndex);

		std::string registerStatusMessage = registerStatus["message"].get<std::string>();
		auto registerStatusMessageBox = new QMessageBox(this);
		registerStatusMessageBox->setWindowTitle("Authorize status");
		registerStatusMessageBox->setText(QString::fromStdString(registerStatusMessage));
		// TODO: position message box on top of the window
		registerStatusMessageBox->show();

		const int showStatusBoxDuration = 1500; // ms
		QTimer::singleShot(showStatusBoxDuration, registerStatusMessageBox, [registerStatusMessageBox]() {
			registerStatusMessageBox->close();
			registerStatusMessageBox->deleteLater();
		});
	});
	// ------------------------------------------------

	// login page

	auto loginLineEdit = new QLineEdit;
	loginLineEdit->setMaximumWidth(lineEditsMaxWidth);
	loginLineEdit->setPlaceholderText("Enter login");

	auto passwordLineEdit = new QLineEdit;
	passwordLineEdit->setMaximumWidth(lineEditsMaxWidth);
	passwordLineEdit->setPlaceholderText("Enter password");
	passwordLineEdit->setEchoMode(QLineEdit::EchoMode::Password);

	auto toRegisterButton = new QPushButton("Register");

	auto vLoginLayout = new QVBoxLayout;
	vLoginLayout->addWidget(loginLineEdit, 0);
	vLoginLayout->addWidget(passwordLineEdit, 0);
	vLoginLayout->addWidget(toRegisterButton);
	vLoginLayout->setAlignment(Qt::AlignCenter);
	_loginPage->setLayout(vLoginLayout);

	auto parseDataFromInput = [this, loginLineEdit, passwordLineEdit]() {
		QString login = loginLineEdit->text().trimmed();
		QString password = passwordLineEdit->text().trimmed();

		if (!login.isEmpty() && !password.isEmpty())
		{
			_client->RequestAuthorize(login.toStdString(), password.toStdString());
		}
	};

	connect(loginLineEdit, &QLineEdit::returnPressed, this, parseDataFromInput);
	connect(passwordLineEdit, &QLineEdit::returnPressed, this, parseDataFromInput);

	connect(_messageObserver.get(), &QtMessageObserver::Authorize, this, [this](const MessageView& message) {
		json authorizeStatus = json::parse(message.Content);

		bool isAuthorized = authorizeStatus["is_authorized"].get<bool>();
		int nextPageIndex = isAuthorized ? 1 : 0; // 1 - main page | 0 - login page
		_pages->setCurrentIndex(nextPageIndex);
		
		std::string authorizeStatusMessage = authorizeStatus["message"].get<std::string>();		
		auto authorizeStatusMessageBox = new QMessageBox(this);
		authorizeStatusMessageBox->setWindowTitle("Authorize status");
		authorizeStatusMessageBox->setText(QString::fromStdString(authorizeStatusMessage));
		// TODO: position message box on top of the window
		authorizeStatusMessageBox->show();

		const int showStatusBoxDuration = 1500; // ms
		QTimer::singleShot(showStatusBoxDuration, authorizeStatusMessageBox, [authorizeStatusMessageBox]() {
			authorizeStatusMessageBox->close();
			authorizeStatusMessageBox->deleteLater();
		});
	});

	connect(toRegisterButton, &QPushButton::clicked, _pages, [this]() {
		_pages->setCurrentIndex(2); // switch page to register
	});

	connect(_messageObserver.get(), &QtMessageObserver::AlreadyAuthorized, _pages, [this]() {
		_pages->setCurrentIndex(1); // skip authorize because session is valid
	});

	// -----------------------------------
	// left side panel widgets
	auto nameLineEdit = new QLineEdit;
	auto userComboBox = new QComboBox;
	auto chatIdComboBox = new QComboBox;
	auto userChatIdComboBox = new PopUpSignalEmittingQComboBox; // TODO change type in the future (if needed)
	userChatIdComboBox->addItem("No chat");
	auto createChatPushButton = new QPushButton("Create chat");
	auto createChatHelperWindow = new HelperWindow(this);
	createChatHelperWindow->SetPlaceholderTextLineEdit("Enter user name to invite in a new chat");
	createChatHelperWindow->hide();

	auto vSidePanelLayout = new QVBoxLayout;
	vSidePanelLayout->addWidget(nameLineEdit);
	vSidePanelLayout->addWidget(new QLabel("All users"));
	vSidePanelLayout->addWidget(userComboBox);
	vSidePanelLayout->addWidget(new QLabel("All chats"));
	vSidePanelLayout->addWidget(chatIdComboBox);
	vSidePanelLayout->addWidget(new QLabel("Your chats"));
	vSidePanelLayout->addWidget(userChatIdComboBox);
	vSidePanelLayout->addWidget(_noticeBox);
	vSidePanelLayout->addWidget(createChatPushButton);
	vSidePanelLayout->addStretch(0);
	// main space
	auto chat = new ChatTextFrame;
	auto messageTextBar = new ChatTextLine(300, 25);
	chat->hide();
	messageTextBar->hide();
	
	auto vMainSpaceLayout = new QVBoxLayout;
	vMainSpaceLayout->addWidget(chat, 0);
	vMainSpaceLayout->addWidget(messageTextBar, 0);
	vMainSpaceLayout->setStretch(0, 5);
	vMainSpaceLayout->setStretch(1, 1);
	auto mainSpaceChatWidget = new QWidget;
	mainSpaceChatWidget->setLayout(vMainSpaceLayout);
	// -----------------------------------

	auto hLayoutMainPage = new QHBoxLayout;
	hLayoutMainPage->addLayout(vSidePanelLayout);
	hLayoutMainPage->addWidget(mainSpaceChatWidget);
	hLayoutMainPage->setStretch(0, 1);
	hLayoutMainPage->setStretch(1, 4);
	hLayoutMainPage->setAlignment(Qt::AlignTop | Qt::AlignLeft);

	_mainPage->setLayout(hLayoutMainPage);

	// TODO: connect widgets
	connect(_messageObserver.get(), &QtMessageObserver::IncomingMessage, chat, [this, chat](const MessageView& messageView) {
		if (chat->GetCurrentChat() == messageView.ChatID)
		{
			auto message = new Message(
				messageView.ID.value(),
				QString::fromStdString(messageView.Author),
				QString::fromStdString(messageView.Content),
				chat
			);
			chat->AddMessage(message);
		}
	});

	connect(_messageObserver.get(), &QtMessageObserver::NewClientName, nameLineEdit, &QLineEdit::setText);
	connect(nameLineEdit, &QLineEdit::returnPressed, [this, nameLineEdit]() {
		std::string desiredIdentity = nameLineEdit->text().toStdString();
		_client->RequestChangeIdentity(desiredIdentity);
	});
	connect(userChatIdComboBox, &PopUpSignalEmittingQComboBox::PoppedUp, [this]() {
		_client->GetClientChatIdsStr();
	});
	connect(userChatIdComboBox, &QComboBox::currentTextChanged, chat, [userChatIdComboBox, messageTextBar, chat](const QString& text) {
		if (userChatIdComboBox->findText(text) == 0)
		{ 
			chat->hide();
			messageTextBar->hide();
		}
		else
		{
			chat->show();
			messageTextBar->show();
			chat->SetCurrentChat(text);
			chat->RemoveMessages();
			// TODO: cache chats messages and update cache on signal, ask for messages in next chat ID if no cache
		}
	});
	connect(_messageObserver.get(), &QtMessageObserver::ClientChats, userChatIdComboBox, [userChatIdComboBox](const MessageView& messageData) {
		try
		{
			json jsonMessageData = json::parse(messageData.Content);

			for (const auto& chatIdJsonValue : jsonMessageData)
			{
				QString chatIdStr = QString::fromStdString(chatIdJsonValue.get<std::string>());

				if (userChatIdComboBox->findText(chatIdStr) == -1)
				{
					userChatIdComboBox->addItem(chatIdStr);
				}
			}
		}
		catch (const json::exception& e)
		{
			qWarning() << e.what();
			return;
		}
	});
	
	connect(createChatPushButton, &QPushButton::clicked, createChatHelperWindow, &QWidget::show);
	connect(createChatHelperWindow, &HelperWindow::TextChanged, [this, createChatHelperWindow](const QString& name) {
		if (!createChatHelperWindow->IsHidden())
		{
			_client->GetClientsByName(name.toStdString());
		}
	});
	connect(_messageObserver.get(), &QtMessageObserver::ClientsByName, createChatHelperWindow, 
	[createChatHelperWindow](const std::string& clientsStr) -> void
	{
		json clientsNamesData = json::parse(clientsStr);

		if (clientsNamesData.empty())
		{
			createChatHelperWindow->HideClientList();
			return;
		}

		QStringList clients;

		try
		{
			for (const auto& client : clientsNamesData)
			{
				clients.push_back(QString::fromStdString(client.get<std::string>()));
			}			
		}
		catch (const json::exception& e)
		{
			qWarning() << "Unable to parse json and create QStringList\n" << e.what();
			return;
		}

		createChatHelperWindow->AddItems(clients);
		createChatHelperWindow->ShowClientList();
	});
	connect(createChatHelperWindow, &HelperWindow::ConfirmClicked, createChatHelperWindow, [this, createChatHelperWindow]() {
		_client->RequestToCreateChat(createChatHelperWindow->GetChosenClientsString());
		createChatHelperWindow->hide();
	});

	connect(_messageObserver.get(), &QtMessageObserver::CreateChat, _noticeBox, [this](const MessageView& messageView) {
		_noticeBox->AddNotification(messageView);
	});
	connect(_noticeBox, &NoticeBox::InvitationProcessed, [this](int chatId, bool isAccepted) {
		std::string dummyInviteResponseString = isAccepted ? "yes" : "no";
		_client->ReplyChatInvite(dummyInviteResponseString);
	});

	connect(messageTextBar, &ChatTextLine::SendedText, [this, chat](const QString& text) {
		std::string stdText = text.toStdString();
		_client->SendMessageToChat(stdText, chat->GetCurrentChat());
	});
}

ChatUI::~ChatUI() {}