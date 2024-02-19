
typedef void (*OnTickExt2_t)(const OnTickExtParameters*);  // alarm callback function typedef

struct OnTickExtParameters
{
    virtual ~OnTickExtParameters() = default; // Virtual destructor
};

class TimeAlarmsClass
{
public:
    void delay();
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



TimeAlarmsClass *Alarm2 = new TimeAlarmsClass();

// with the following I can get a pointer to the object created on the stack?
TimeAlarmsClass Alarm1 = TimeAlarmsClass();
void test() {
    TimeAlarmsClass *Alarm3 = &Alarm1;
}

void test() {
  Alarm1.delay();
  (*Alarm2).delay();
}

// if i run the following on ESP8266 with a stack of 4096 bytes
// will it then have stack overflow exception?
class Test {
    char buff[4097];
    Test();
};

/*void main(void) {
    Test test;

}*/
