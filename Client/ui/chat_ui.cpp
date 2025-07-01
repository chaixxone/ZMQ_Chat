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
	// TODO: remove dummy message
	QString dummyText = "Lorem ipsum dolor sit amet, consectetur adipiscing elit,"
		"sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim"
		"ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip"
		"ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate"
		"velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat"
		"cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
	auto message = new Message(564755748ull, "chaixxone", dummyText, chat);
	chat->AddMessage(message);
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
}

ChatUI::~ChatUI() {}