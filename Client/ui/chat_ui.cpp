#include <chat_ui.hpp>
#include <chat_text_frame.hpp>
#include <chat_text_line.hpp>

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
		auto message = new Message(
			std::stoull(messageView.ID),
			QString::fromStdString(messageView.Author),
			QString::fromStdString(messageView.Content),
			chat
		);
		chat->AddMessage(message);
	});

	connect(_messageObserver.get(), &QtMessageObserver::NewClientName, nameLineEdit, &QLineEdit::setText);	
}

ChatUI::~ChatUI() {}