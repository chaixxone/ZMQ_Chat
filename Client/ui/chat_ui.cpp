#include <chat_ui.hpp>
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
	auto chatIdComboBox = new PopUpSingalEmittingQComboBox; // TODO change type in the future (if needed)
	auto userChatIdComboBox = new QComboBox;

	auto vSidePanelLayout = new QVBoxLayout;
	vSidePanelLayout->addWidget(nameLineEdit);
	vSidePanelLayout->addWidget(userComboBox);
	vSidePanelLayout->addWidget(chatIdComboBox);
	vSidePanelLayout->addWidget(userChatIdComboBox);
	vSidePanelLayout->addStretch(0);
	// main space
	auto chat = new ChatTextFrame;
	auto messageTextBar = new ChatTextLine(300, 25);
	
	auto vMainSpaceLayout = new QVBoxLayout;
	vMainSpaceLayout->addWidget(chat, 0);
	vMainSpaceLayout->addWidget(messageTextBar, 0);
	vMainSpaceLayout->setStretch(0, 5);
	vMainSpaceLayout->setStretch(1, 1);
	// -----------------------------------

	auto hLayoutMainPage = new QHBoxLayout;
	hLayoutMainPage->addLayout(vSidePanelLayout);
	hLayoutMainPage->addLayout(vMainSpaceLayout);
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
	connect(chatIdComboBox, &PopUpSingalEmittingQComboBox::PoppedUp, [client = _client]() {
		client->GetClientChatIdsStr();
	});
	connect(chatIdComboBox, &QComboBox::currentTextChanged, chat, &ChatTextFrame::SetCurrentChat);
	connect(_messageObserver.get(), &QtMessageObserver::ClientChats, userChatIdComboBox, [userChatIdComboBox](const MessageView& messageData) {
		try
		{
			json jsonMessageData = json::parse(messageData.Content);
			json& chatIdsJsonArray = jsonMessageData[0];

			for (const auto& chatIdJsonValue : chatIdsJsonArray)
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
	connect(messageTextBar, &ChatTextLine::SendedText, [this](const QString& text) {
		std::string stdText = text.toStdString();
		_client->SendMessageToChat(stdText, _client->GetChatId());
	});
}

ChatUI::~ChatUI() {}