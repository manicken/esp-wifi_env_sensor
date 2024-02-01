
typedef void (*OnTickExt2_t)(const OnTickExtParameters*);  // alarm callback function typedef

struct OnTickExtParameters
{
    virtual ~OnTickExtParameters() = default; // Virtual destructor
};

class TimeAlarmsClass
{
public:

   int create(OnTickExt2_t, OnTickExtParameters*){  // alarm callback function typedef);
   
   }
};

struct AsIntParameter : public OnTickExtParameters
{
    AsIntParameter(int value): value(value) {}
    int value = 0;
};

void Test(OnTickExtParameters *params)
{

}

int main()
{
	TimeAlarmsClass tac;

	AsIntParameter *params = new AsIntParameter(42);
	tac.create(Test, params);
	
}