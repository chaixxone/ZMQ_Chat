#include <chat_ui.hpp>
#include <qt_helper_window.hpp>
#include <chat_text_frame.hpp>
#include <chat_text_line.hpp>
#include <popup_signal_emitting_q_combo_box.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace UI;

ChatUI::ChatUI(std::shared_ptr<Client> client, std::shared_ptr<QtMessageObserver> observer, QWidget* parent) :
	QMainWindow(parent), 
	_pages(new QStackedWidget(this)), 
	_loginPage(new QWidget), 
	_mainPage(new QWidget),
	_client(client),
	_messageObserver(observer)
{
	_pages->addWidget(_loginPage);
	_pages->addWidget(_mainPage);
	// TODO: remove current index setting
	_pages->setCurrentIndex(1);
	setCentralWidget(_pages);

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

	connect(messageTextBar, &ChatTextLine::SendedText, [this, chat](const QString& text) {
		std::string stdText = text.toStdString();
		_client->SendMessageToChat(stdText, chat->GetCurrentChat());
	});
}

ChatUI::~ChatUI() {}