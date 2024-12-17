/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "oled.h"
#include "font.h" 
#include "UI_Move.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef struct Menutypedef;
typedef struct ItemTypedef;

typedef struct Menutypedef
{
  char *menuName;
  ItemTypedef *itemList;
  Menutypedef *parentMenu; // 父级菜单指针
  uint16_t itemCount;
}Menutypedef;

typedef struct ItemTypedef
{
  char *str;
  uint8_t len;
  void (*Function)(void);
  Menutypedef *subMenu; // 子级菜单指针

}ItemTypedef;


typedef struct 
{
  int16_t val;
  int16_t targetVal;
  int16_t lastVal;

}UIContextTypedef;


typedef struct 
{
  uint8_t val;
  uint8_t lastVal;
  uint8_t updateFlag;
}KeyTypdef;


typedef struct
{
  int16_t topIndex;
  int16_t bottomIndex;
}ScreenIndexTypedef;


typedef enum 
{
    INIT                    = 0x01,
    X_MOVING                = 0x02,          
    GEN_CHAR                = 0x03,
    SEARCH_FINGERPRINT      = 0x04,
    SUCCESS_USER            = 0x05,
    FAILURE                 = 0x06
} UIMoveStateTypedef;



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define LIMIT_MAGNITUDE(value, low, high) \
        ((value) < (low) ? (low) : ((value) > (high) ? (high) : (value)))

#define OLED_SCREEN_WIDTH   128
#define OLED_SCREEN_HEIGHT  64
#define MENU_ITEM_HEIGHT    16

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
ItemTypedef meanItemList[] = 
{
  {"List", 4, NULL, NULL},
  {"a", 1, NULL, NULL},
  {"wqweqw", 6, NULL, NULL},
  {"sds", 3, NULL, NULL},
  {"List_E", 6, NULL, NULL},
  {"afsadf", 6, NULL, NULL},
  {"wode", 4, NULL, NULL},
  {"List", 4, NULL, NULL},
  {"a", 1, NULL, NULL},
  {"wqweqw", 6, NULL, NULL},
  {"sds", 3, NULL, NULL},
  {"List_E", 6, NULL, NULL},
  {"afsadf", 6, NULL, NULL},
  {"wode", 4, NULL, NULL}

  
};

KeyTypdef KeyList[2] = {0};

UIContextTypedef itemX = {0};
UIContextTypedef itemY = {0};

UIContextTypedef frameWidth = {0};
UIContextTypedef frameY = {0};

UIContextTypedef screenTop = {0};
UIContextTypedef screenBottom = {0};
ScreenIndexTypedef screenIndex = {0, 3};

// 0:没有按键响应 1:KEY1 2:KEY2
uint8_t keyID = 0;
int8_t itemIndex = 0;
uint16_t listLen = 0;
float moveProcess_Frame = 0.0;
float moveProcess_Screen = 0.0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}
int fgetc(FILE *f)
{
  uint8_t ch = 0;
  HAL_UART_Receive(&huart1, &ch, 1, 0xffff);
  return ch;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_0)
  {
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
    {
      KeyList[0].val = 1;
      keyID = 1;
    }
    else 
    {
      KeyList[0].val = 0;
      keyID = 0;
    }

    if (KeyList[0].val != KeyList[0].lastVal)
    {
      KeyList[0].lastVal = KeyList[0].val;
      KeyList[0].updateFlag = 1;
    }
  }
  else if (GPIO_Pin == GPIO_PIN_13)
  {
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET)
    {
      KeyList[1].val = 1;
      keyID = 2;
    }
    else 
    {
      KeyList[1].val = 0;
      keyID = 0;
    }

    if (KeyList[1].val != KeyList[1].lastVal)
    {
      KeyList[1].lastVal = KeyList[1].val;
      KeyList[1].updateFlag = 1;
    }
  }
}

// 返回值： 1： 到达目标哦位置； 0： 没有到达目标位置
uint8_t Frame_Move(UIContextTypedef *frameY, UIContextTypedef *frameWidth, float moveSpeed);
uint8_t Screen_Move(UIContextTypedef *screenTop, float moveSpeed);
void Key_Init();
void ItemIndex_UpData();
void Frame_UpData();
void Screen_UpData();
void UI_Show();
float easeInOut(float t);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  Key_Init();
  HAL_Delay(20);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("ready:)\n");
  frameY.targetVal = itemIndex * MENU_ITEM_HEIGHT;
  frameWidth.targetVal = itemList[itemIndex].len * 6 + 4;
  frameWidth.val = itemList[itemIndex].len * 6 + 4;

  while (1)
  {
    listLen = sizeof(itemList) / sizeof(ItemTypedef);
    ItemIndex_UpData();
    Frame_Move(&frameY, &frameWidth, 0.1);
    Screen_Move(&screenTop, 0.1);
    UI_Show();
    // printf("%d\n", itemIndex);
       
    // printf("%d\n", xMove);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


void Key_Init()
{
  memset(KeyList, 0, sizeof(KeyList));
}



void ItemIndex_UpData()
{
  if (KeyList[keyID - 1].updateFlag && KeyList[keyID - 1].val && keyID)
  {
    
    KeyList[keyID - 1].updateFlag = 0;
    if (keyID == 1)
    {
      itemIndex++;
      
    }
    else if (keyID == 2)
    {
      itemIndex--;
    }

    if (itemIndex == listLen)
    {
      itemIndex = 0;
    }
    else if (itemIndex == -1)
    {
      itemIndex = listLen - 1;
    }
    

    Frame_UpData();
    Screen_UpData();
  }
}

void Frame_UpData()
{
  moveProcess_Frame = 0.0;
  frameY.lastVal = frameY.targetVal;
  frameWidth.lastVal = frameWidth.targetVal;
  
  frameY.targetVal = itemIndex * MENU_ITEM_HEIGHT;
  frameWidth.targetVal = itemList[itemIndex].len * 6 + 4;
}

void Screen_UpData()
{
  if (itemIndex > screenIndex.bottomIndex)
  {
    moveProcess_Screen = 0.0;
    screenIndex.bottomIndex = itemIndex;
    screenIndex.topIndex = screenIndex.bottomIndex - 3;
  }
  else if (itemIndex < screenIndex.topIndex)
  {
    moveProcess_Screen = 0.0;
    screenIndex.topIndex = itemIndex;
    screenIndex.bottomIndex = screenIndex.topIndex + 3;
  }

  screenTop.lastVal = screenTop.targetVal;

  screenTop.targetVal = screenIndex.topIndex * MENU_ITEM_HEIGHT;

}

uint8_t Screen_Move(UIContextTypedef *screenTop, float moveSpeed)
{
  if (moveProcess_Screen < 1.0)
  {
    moveProcess_Screen += moveSpeed;
  }
  if (moveProcess_Screen > 1.0)
  {
    moveProcess_Screen = 1.0;
  }

  float easedProcess = easeInOut(moveProcess_Screen);

  screenTop->val = (int16_t)((1.0 - easedProcess) * screenTop->lastVal + easedProcess * screenTop->targetVal);

  if (moveProcess_Screen == 1.0)
  {
    screenTop->val = screenTop->targetVal;
    return 1;
  }
  else
  {
    return 0;
  }
}

uint8_t Frame_Move(UIContextTypedef *frameY, UIContextTypedef *frameWidth, float moveSpeed)
{

  if (moveProcess_Frame < 1.0)
  {
    moveProcess_Frame += moveSpeed;
  }
  if (moveProcess_Frame > 1.0)
  {
    moveProcess_Frame = 1.0;
  }

  float easedProcess = easeInOut(moveProcess_Frame);

  frameY->val = (int16_t)((1.0 - easedProcess) * frameY->lastVal + easedProcess * frameY->targetVal);
  frameWidth->val = (int16_t)((1.0 - easedProcess) * frameWidth->lastVal + easedProcess * frameWidth->targetVal);

  // frameY->val += (int16_t)((frameY->targetVal - frameY->val) * moveProcess);
  // frameWidth->val += (int16_t)((frameWidth->targetVal - frameWidth->val) * moveProcess);

  if (moveProcess_Frame == 1.0)
  {
    frameY->val = frameY->targetVal;
    frameWidth->val = frameWidth->targetVal;
    return 1;
  }
  else
  {
    return 0;
  }

}

void UI_Show()
{
  OLED_NewFrame();
  printf("%d, %d\n", screenTop.val, frameY.val);
  for (int i = -1; i < 5; i++)
  {
    OLED_PrintASCIIString(2, screenTop.targetVal - screenTop.val + (i * MENU_ITEM_HEIGHT) + 4, itemList[i + screenIndex.topIndex].str, &afont8x6, OLED_COLOR_NORMAL); 
  }
  OLED_DrawFilledRectangle(0, frameY.val + 1 - (screenIndex.topIndex * MENU_ITEM_HEIGHT), frameWidth.val, MENU_ITEM_HEIGHT - 2, OLED_COLOR_REVERSED); 
  OLED_ShowFrame();
}

float easeInOut(float t) 
{
  if (t < 0.5)
    return 2 * t * t;
  else
    return -1 + (4 - 2 * t) * t;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
