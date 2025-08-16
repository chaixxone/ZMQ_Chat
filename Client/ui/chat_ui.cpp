#include <chat_ui.hpp>
#include <chat_invite.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace UI;

enum Pages
{
	LoginPage    = 0,
	MainPage     = 1,
	RegisterPage = 2
};

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
	_pages->setCurrentIndex(Pages::LoginPage);
	setCentralWidget(_pages);	

	SetupRegisterPage();
	SetupLoginPage();
	SetupMainPage();

	connect(_messageObserver.get(), &QtMessageObserver::AlreadyAuthorized, _pages, [this]() {
		_pages->setCurrentIndex(Pages::MainPage);
	});

	connect(_messageObserver.get(), &QtMessageObserver::NotAuthorized, _pages, [this](const MessageView& message) {
		_pages->setCurrentIndex(Pages::LoginPage);

		auto messageBox = new QMessageBox(this);
		messageBox->setWindowTitle("Not authorized");
		messageBox->setText(QString::fromStdString(message.Content));
		// TODO: position message box on top of the window
		messageBox->show();
		QPoint chatUITopLeft = this->geometry().topLeft();
		int x = chatUITopLeft.x() + (this->width() - messageBox->width()) / 2;
		int y = chatUITopLeft.y();
		messageBox->move(x, y);

		const int showStatusBoxDuration = 1500; // ms
		QTimer::singleShot(showStatusBoxDuration, messageBox, [messageBox]() {
			messageBox->close();
			messageBox->deleteLater();
		});
	});
}

ChatUI::~ChatUI() {}

void ChatUI::SetupRegisterPage()
{
	auto registerLoginLineEdit = new QLineEdit;
	registerLoginLineEdit->setMaximumWidth(_lineEditsMaxWidth);
	registerLoginLineEdit->setPlaceholderText("Enter login");

	auto registerPasswordLineEdit = new QLineEdit;
	registerPasswordLineEdit->setMaximumWidth(_lineEditsMaxWidth);
	registerPasswordLineEdit->setPlaceholderText("Enter password");
	registerPasswordLineEdit->setEchoMode(QLineEdit::EchoMode::Password);

	auto registerPasswordRepeatLineEdit = new QLineEdit;
	registerPasswordRepeatLineEdit->setMaximumWidth(_lineEditsMaxWidth);
	registerPasswordRepeatLineEdit->setPlaceholderText("Repeat password");
	registerPasswordRepeatLineEdit->setEchoMode(QLineEdit::EchoMode::Password);

	auto toLoginButton = new QPushButton("Login", _registerPage);
	connect(toLoginButton, &QPushButton::clicked, _pages, [this]() {
		_pages->setCurrentIndex(Pages::LoginPage);
	});

	auto vRegisterLayout = new QVBoxLayout;
	vRegisterLayout->addStretch(0);
	vRegisterLayout->addWidget(registerLoginLineEdit, 0, Qt::AlignCenter);
	vRegisterLayout->addWidget(registerPasswordLineEdit, 0, Qt::AlignCenter);
	vRegisterLayout->addWidget(registerPasswordRepeatLineEdit, 0, Qt::AlignCenter);
	vRegisterLayout->addStretch(0);
	vRegisterLayout->addWidget(toLoginButton, 0, Qt::AlignBottom | Qt::AlignLeft);
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

	connect(_messageObserver.get(), &QtMessageObserver::Register, this, &ChatUI::OnRegisterAction);
}

void ChatUI::OnRegisterAction(const MessageView& message)
{
	json registerStatus = json::parse(message.Content);
	bool isRegistered = registerStatus["is_registered"].get<bool>();

	// TODO take login and password and paste them into login page inputs

	int nextPageIndex = isRegistered ? Pages::LoginPage : Pages::RegisterPage;

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
}

void ChatUI::SetupLoginPage()
{
	auto loginLineEdit = new QLineEdit;
	loginLineEdit->setMaximumWidth(_lineEditsMaxWidth);
	loginLineEdit->setPlaceholderText("Enter login");

	auto passwordLineEdit = new QLineEdit;
	passwordLineEdit->setMaximumWidth(_lineEditsMaxWidth);
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

	connect(toRegisterButton, &QPushButton::clicked, _pages, [this]() {
		_pages->setCurrentIndex(Pages::RegisterPage);
	});

	connect(loginLineEdit, &QLineEdit::returnPressed, this, parseDataFromInput);
	connect(passwordLineEdit, &QLineEdit::returnPressed, this, parseDataFromInput);

	connect(_messageObserver.get(), &QtMessageObserver::Authorize, this, &ChatUI::OnAuthorizeAction);
}

void ChatUI::OnAuthorizeAction(const MessageView& message)
{
	json authorizeStatus = json::parse(message.Content);

	bool isAuthorized = authorizeStatus["is_authorized"].get<bool>();
	int nextPageIndex = isAuthorized ? Pages::MainPage : Pages::LoginPage;
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
}

void ChatUI::SetupMainPage()
{
	QLayout* vSidePanelLayout = SetupSidePanel();

	_chat = new ChatTextFrame;
	_chat->setObjectName("chat_frame");

	_messageTextBar = new ChatTextLine(300, 25);
	_messageTextBar->setObjectName("text_bar");

	_chat->hide();
	_messageTextBar->hide();

	auto vMainSpaceLayout = new QVBoxLayout;
	vMainSpaceLayout->addWidget(_chat, 0);
	vMainSpaceLayout->addWidget(_messageTextBar, 0);
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

	ConnectAllSignals();
}

QLayout* ChatUI::SetupSidePanel()
{
	_nameLineEdit = new QLineEdit;
	_nameLineEdit->setObjectName("name_line");

	_userChatIdComboBox = new PopUpSignalEmittingQComboBox; // TODO change type in the future (if needed)
	_userChatIdComboBox->setObjectName("user_chats");
	_userChatIdComboBox->addItem("No chat");
	
	_createChatPushButton = new QPushButton("Create chat");
	_createChatPushButton->setObjectName("create_chat_button");

	_createChatHelperWindow = new HelperWindow(this);
	_createChatHelperWindow->setObjectName("create_chat_window");
	_createChatHelperWindow->SetPlaceholderTextLineEdit("Enter user name to invite in a new chat");
	_createChatHelperWindow->hide();

	auto logoutButton = new QPushButton("Logout");
	connect(logoutButton, &QPushButton::clicked, this, [this]() { _client->RequestLogout(); });

	auto vSidePanelLayout = new QVBoxLayout;
	vSidePanelLayout->addWidget(_nameLineEdit);
	vSidePanelLayout->addWidget(new QLabel("Your chats"));
	vSidePanelLayout->addWidget(_userChatIdComboBox);
	vSidePanelLayout->addWidget(_noticeBox);
	vSidePanelLayout->addWidget(_createChatPushButton);
	vSidePanelLayout->addStretch();
	vSidePanelLayout->addWidget(logoutButton, 0, Qt::AlignLeft);

	return vSidePanelLayout;
}

void ChatUI::ConnectAllSignals()
{
	connect(_messageObserver.get(), &QtMessageObserver::NewClientName, _nameLineEdit, &QLineEdit::setText);
	connect(_nameLineEdit, &QLineEdit::returnPressed, [this]() {
		std::string desiredIdentity = _nameLineEdit->text().toStdString();
		_client->RequestChangeIdentity(desiredIdentity);
	});

	ConnectSignalsCreateChat();
	ConnectSignalsUserChats();
	ConnectChatMessageSignals();
	ConnectNoticeBoxSignals();
}

void ChatUI::ConnectSignalsCreateChat()
{
	connect(_createChatPushButton, &QPushButton::clicked, _createChatHelperWindow, &QWidget::show);
	connect(_createChatHelperWindow, &HelperWindow::TextChanged, [this](const QString& name) {
		if (!_createChatHelperWindow->IsHidden())
		{
			_client->GetClientsByName(name.toStdString());
		}
	});
	connect(_messageObserver.get(), &QtMessageObserver::ClientsByName, _createChatHelperWindow, [this](const std::string& clientsStr)
	{
		json clientsNamesData = json::parse(clientsStr);

		if (clientsNamesData.empty())
		{
			_createChatHelperWindow->HideClientList();
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

		_createChatHelperWindow->AddItems(clients);
		_createChatHelperWindow->ShowClientList();
	});
	connect(_createChatHelperWindow, &HelperWindow::ConfirmClicked, _createChatHelperWindow, [this]() {
		_client->RequestToCreateChat(_createChatHelperWindow->GetChosenClientsString());
		_createChatHelperWindow->hide();
	});
}

void ChatUI::ConnectSignalsUserChats()
{
	connect(_userChatIdComboBox, &PopUpSignalEmittingQComboBox::PoppedUp, [this]() {
		_client->GetClientChatIdsStr();
	});
	connect(_userChatIdComboBox, &QComboBox::currentTextChanged, _chat, [this](const QString& text) {
		if (_userChatIdComboBox->findText(text) == 0)
		{
			_chat->hide();
			_messageTextBar->hide();
		}
		else
		{
			_chat->show();
			_messageTextBar->show();
			_chat->SetCurrentChat(text);
			_chat->RemoveMessages();
			// TODO: cache chats messages and update cache on signal, ask for messages in next chat ID if no cache
		}
	});
	connect(_messageObserver.get(), &QtMessageObserver::ClientChats, _userChatIdComboBox, [this](const MessageView& messageData) {
		try
		{
			json jsonMessageData = json::parse(messageData.Content);

			for (const auto& chatIdJsonValue : jsonMessageData)
			{
				QString chatIdStr = QString::number(chatIdJsonValue.get<int>());

				if (_userChatIdComboBox->findText(chatIdStr) == -1)
				{
					_userChatIdComboBox->addItem(chatIdStr);
				}
			}
		}
		catch (const json::exception& e)
		{
			qWarning() << e.what();
			return;
		}
	});
}

void ChatUI::ConnectChatMessageSignals()
{
	connect(_messageObserver.get(), &QtMessageObserver::IncomingMessage, _chat, [this](const MessageView& messageView) {
		if (_chat->GetCurrentChat() == messageView.ChatID)
		{
			auto message = new Message(
				messageView.ID.value(),
				QString::fromStdString(messageView.Author),
				QString::fromStdString(messageView.Content),
				_chat
			);
			_chat->AddMessage(message, messageView.Author == _client->GetIdentity());
		}
	});

	connect(_messageTextBar, &ChatTextLine::SendedText, [this](const QString& text) {
		std::string stdText = text.toStdString();
		_client->SendMessageToChat(stdText, _chat->GetCurrentChat());
	});
}

void ChatUI::ConnectNoticeBoxSignals()
{
	connect(_messageObserver.get(), &QtMessageObserver::CreateChat, _noticeBox, [this](const MessageView& messageView) {
		_noticeBox->ProcessNotification(messageView);
	});
	connect(_noticeBox, &NoticeBox::InvitationProcessed, [this](int notificationID, int chatId, bool isAccepted) {
		_client->ReplyChatInvite(chatId, notificationID, isAccepted);
	});
	connect(_noticeBox, &NoticeBox::FetchAllNotifications, this, [this]() {
		_client->GetNotifications();
	});
	connect(_messageObserver.get(), &QtMessageObserver::Notifications, _noticeBox, [this](const MessageView& messageView) {
		_noticeBox->ProcessAllNotifications(messageView);
	});
}