#include "Game.hpp"

#include <iostream>
#include <functional>

Game::Game() :	window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Coin Game"),
				numPiles(NUM_PILES),
				p1(PLAYER_1_HUMAN),
				p2(PLAYER_2_HUMAN)
{

	auto initializeText = [](const sf::Font& font, int charSize, const std::string str) {
		sf::Text text;
		text.setFont(font);
		text.setCharacterSize(charSize);
		text.setString(str);
		return text;
	};

	srand(static_cast<int>(time(NULL)));

	//Loading fonts and setting them to all the text
	if (font.loadFromFile("media/font/retganon.ttf")) {
		for (int i = 0; i < 3; ++i) pileText.push_back(initializeText(font, 30, ""));
		for (auto prime : PRIMES) primesText.push_back(initializeText(font, 30, std::to_string(prime)));
		winText = initializeText(font, 50, "");
		playerInfoText = initializeText(font, 35, "");
	} else {
		std::cout << "Error reading font from media/font/ - needs trs-million.ttf font\n";
	}

	//Loading texture
	if (!coinTexture.loadFromFile("media/texture/goldCoin.png")) {
		std::cout << "Error reading texture from media/texture/ - needs goldCoin.png\n";
	}

	if (!backgroundTexture.loadFromFile("media/texture/background.jpg")) {
		std::cout << "Error reading texture from media/texture/ - needs background.jpg\n";
	}

	init();
	render();
}

void Game::init() {
	piles.clear();
	for (int i = 0; i < numPiles; ++i) piles.push_back((rand() % COINS_PER_PILE) + 1);
	current = &p1;
	pileText[0].setPosition(150, WINDOW_HEIGHT / 3);
	pileText[1].setPosition(390, WINDOW_HEIGHT / 3);
	pileText[2].setPosition(640, WINDOW_HEIGHT / 3);

	float xPos{ WINDOW_WIDTH / 5 };
	for (auto& prime : primesText) {
		prime.setPosition({ xPos, WINDOW_HEIGHT / 1.75f });
		xPos += WINDOW_WIDTH / 15;
	}

	background.setTexture(backgroundTexture);
	float scaleX = (float)window.getSize().x / backgroundTexture.getSize().x;
	float scaleY = (float)window.getSize().y / backgroundTexture.getSize().y;
	background.setScale({ scaleX, scaleY });
}

void Game::run() {
	sf::Clock clock;

	while (window.isOpen()) {
		if (!processEvents()) break;
		update(clock.restart());
		render();
	}
}

bool Game::gameWon() {
	for (auto pile : piles) {
		if (pile > 0) return false;
	}
	return true;
}


bool Game::processEvents() {

	sf::Event event;
	while (window.pollEvent(event)) {
		if (event.type == sf::Event::Closed) {
			window.close();
			break;
		}

		if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::R) {
			init();
			return true;
		}
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) return false;

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		sf::Vector2f mouse{ sf::Mouse::getPosition(window) };
		sf::FloatRect textRect;

		for (int pile = 0; pile < pileText.size(); ++pile) {
			sf::FloatRect textRect;
			pileText[pile].getTransform().transformRect(textRect);
			textRect.top = pileText[pile].getPosition().y;
			textRect.left = pileText[pile].getPosition().x;
			textRect.width = 30;
			textRect.height = 30;
			if (textRect.contains(mouse)) {
				selectedPile = pile;
				pileText[pile].setFillColor(sf::Color::Green);
				for (int pile = 0; pile < pileText.size(); ++pile) {
					if (selectedPile != pile) pileText[pile].setFillColor(sf::Color::White);
				}
				break;
			}
		}

		if (selectedPile == -1) return true;
		for (auto& text : primesText) text.setFillColor(sf::Color(0,0,0,0));
		for (int prime = 0; prime < primesText.size(); ++prime) {
			int primeValue = *std::next(PRIMES.begin(), prime);
			if (piles[selectedPile] - primeValue < 0) break;
			primesText[prime].setFillColor(sf::Color::White);
			primesText[prime].getTransform().transformRect(textRect);

			textRect.top = primesText[prime].getPosition().y;
			textRect.left = primesText[prime].getPosition().x;
			textRect.width = 35;
			textRect.height = 35;

			if (textRect.contains(mouse)) {
				primesText[prime].setFillColor(sf::Color::Green);
				selectedPrime = primeValue;
			}
		}
	}
	return true;
}

void Game::update(sf::Time deltaTime) {
	if (gameWon()) return;

	if (!current->isHuman()) {
		current->takeTurn(piles);
		current = current == &p1 ? &p2 : &p1;
		return;
	}
	if (selectedPile < 0 || selectedPile >= numPiles) return;
	if (PRIMES.count(selectedPrime) == 0) return;

	int diff = piles[selectedPile] - selectedPrime;
	if (diff >= 0) {
		piles[selectedPile] = diff;
		current = current == &p1 ? &p2 : &p1;
		selectedPile = -1;
		selectedPrime = -1;
		for (auto& text : pileText) text.setFillColor(sf::Color::White);
		for (auto& text : primesText) text.setFillColor(sf::Color(0, 0, 0, 0));
		return;
	}
	//std::cout << "You entered something wrong!" << std::endl;
}

void Game::render() {
	window.clear(sf::Color::Black);
	window.draw(background);

	if (gameWon()) {
		std::string winner = current == &p1 ? "2" : "1";
		winText.setString("Player " + winner + " Has Won!");
		winText.setPosition({WINDOW_WIDTH/3, WINDOW_HEIGHT/2});
		window.draw(winText);
		window.display();
		return;
	}

	std::function<void(sf::Vector2f, int, int)> drawPile;
	drawPile = [this, &drawPile](sf::Vector2f startLocation, int amount, int row) {
		if (amount <= 0) return;
		sf::RectangleShape coinShape({ WINDOW_WIDTH / 29, WINDOW_HEIGHT / 60 });
		coinShape.setTexture(&coinTexture);
		coinShape.setFillColor(sf::Color::Yellow);
		for (int r = 0; r < row; ++r) {
			if (amount <= 0) return;
			coinShape.setPosition({ startLocation.x + (coinShape.getSize().x * 1.2f)*r, startLocation.y });
			window.draw(coinShape);
			--amount;
		}
		drawPile({ startLocation.x + coinShape.getSize().x/1.5f, startLocation.y - coinShape.getSize().y*1.5f }, amount, --row);
	};

	auto leadingSpace{ 0 };
	auto pileNum{ 0 };
	for (auto& p : piles) {
		drawPile({(leadingSpace++ * 120.0f) + 50, WINDOW_WIDTH/5 }, p, 7);
		leadingSpace++;

		pileText[pileNum].setString(std::to_string(p));
		window.draw(pileText[pileNum]);
		pileNum++;
	}

	for (auto& prime : primesText) {
		window.draw(prime);
	}

	std::string playerNum = current == &p1 ? "1" : "2";
	playerInfoText.setString("Turn : Player " + playerNum);
	window.draw(playerInfoText);

	window.display();

}

