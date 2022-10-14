/*
 * main.cpp
 *
 *  Created on: Aug 11, 2020
 *      Author: ryu
 */

#include <cstdint>
#include "main.h"
#include "CanClass.hpp"

namespace
{

	constexpr float DelayTime = 0.001;	//値を小さくするとステッピングモーターの回転速度が速くなる

	namespace Command
	{
		enum Command : std::uint8_t
		{
			open = 0,
			close,
			disable,
			enable,

			non_receive
		};
	}


	CanClass can;
	volatile Command::Command receive_data = Command::non_receive;
	int count = 0;
}

//受信
extern "C" {
	void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
	{
		can.receive(receive_data, 0x500);
		can.led_on();
		can.endit();//割り込み終了
	}
};


void main_cpp()
{
	can.init(0x500, 1000000);
	//enable
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);

	while(true)
	{
		HAL_Delay(1000);
		HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
		HAL_GPIO_TogglePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin);

		//問題が起きた時などに手動でdisableにする
		if(receive_data == Command::disable)
		{
			//disable
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
			// 読み取り完了
			receive_data = Command::non_receive;
			count = 0;
		}

		//手動でenableにする
		else if(receive_data == Command::enable)
		{
			//enable
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
			// 読み取り完了
			receive_data = Command::non_receive;
			count = 0;
		}

		//1回目に開く
		else if(receive_data == Command::open && count == 0)
		{
			HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
			//direction
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
			for(int i = 0; i < 1200 ; i++)	//1828パルスはステッピングモーター1/4回転分
			{
				//パルス
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
				HAL_Delay(DelayTime);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
				HAL_Delay(DelayTime);
			}
			receive_data = Command::non_receive;
			count++;
		}
		//2回目以降に開く
		else if(receive_data == Command::open && count != 0 && count % 2 == 0)
		{
			HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
			//direction
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
			for(int i = 0; i < 1200 ; i++)
			{
				//パルス
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
				HAL_Delay(DelayTime);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
				HAL_Delay(DelayTime);
			}
			receive_data = Command::non_receive;
			count++;
		}
		//閉じる
		else if(receive_data == Command::close && count != 0 && count % 2 == 1)
		{
			HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_SET);

			//direction
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);

			for(int i = 0; i < 1200 ; i++)
			{
				//パルス
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
				HAL_Delay(DelayTime);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
				HAL_Delay(DelayTime);
			}
			receive_data = Command::non_receive;
			count++;
		}
		can.led_process();
	}

}
