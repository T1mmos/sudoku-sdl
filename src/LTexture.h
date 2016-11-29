//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		int printMsg(char const * msg);

	private:
		int mHeight;
};