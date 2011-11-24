class LoadScreen : public Scene {
private:
	Graphics::Image Background, StatusString[9];

public:
	int Logic(int Message, int ParamA, int ParamB);
	int Render();

	enum Message {
		msg_MaxisLogo = 0x00010000,
		msg_SlidingText,
		msg_JobComplete,
		msg_LoginSuccessful
	};
}