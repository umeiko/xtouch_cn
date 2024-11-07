#ifndef _CST820_H
#define _CST820_H

#include <Wire.h>

#define I2C_ADDR_CST820 0x15

// 触摸旋转方向
typedef enum {
    Rotation_0 = 0, 
    Rotation_1, 
    Rotation_2,
    Rotation_3, 
} TouchRotation; 


//手势
enum GESTURE
{
    None = 0x00,       //无手势
    SlideDown = 0x01,  //向下滑动
    SlideUp = 0x02,    //向上滑动
    SlideLeft = 0x03,  //向左滑动
    SlideRight = 0x04, //向右滑动
    SingleTap = 0x05,  //单击
    DoubleTap = 0x0B,  //双击
    LongPress = 0x0C   //长按
};

class TS_Point {
public:
    TS_Point(void) : x(0), y(0) {}
    TS_Point(int16_t x, int16_t y) : x(x), y(y) {}
	int16_t x, y;
};

/**************************************************************************/
/*!
    @brief  CST820 I2C CTP controller driver
*/
/**************************************************************************/
class CST820
{
public:
    CST820(int8_t sda_pin = -1, int8_t scl_pin = -1, int8_t rst_pin = -1, int8_t int_pin = -1);

    void begin(void);
    void setRotation(int _rotate);
    bool touched();
    bool tirqTouched();
    TS_Point getPoint();
    bool getTouch(uint16_t *x, uint16_t *y, uint8_t *gesture);
    uint8_t rotate = 1;
private:
    int8_t _sda, _scl, _rst, _int;

    uint8_t i2c_read(uint8_t addr);
    uint8_t i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length);
    void i2c_write(uint8_t addr, uint8_t data);
    uint8_t i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length);
};
#endif