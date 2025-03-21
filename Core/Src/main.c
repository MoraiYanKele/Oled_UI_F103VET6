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

struct Menutypedef ;
struct ItemTypedef ;

typedef struct Menutypedef
{
  char *menuName;
  struct ItemTypedef *items;
  struct Menutypedef *parentMenu; // 父级菜单指针
  uint16_t itemCount;
  int currentItemIndex;

}Menutypedef;

typedef struct ItemTypedef
{
  char *str;
  uint8_t len;
  void (*Function)(void);
  struct Menutypedef *subMenu; // 子级菜单指针

}ItemTypedef;


typedef struct 
{
  int16_t val;
  int16_t targetVal;
  int16_t lastVal;

}UIElemTypedef;


typedef struct {
  uint8_t val;            // 当前值
  uint8_t lastVal;        // 上一次值
  uint8_t updateFlag;     // 更新标志
  uint8_t shortPress;     // 短按标志
  uint8_t longPress;      // 长按标志
  uint32_t pressTime;     // 按下时间
  uint32_t releaseTime;   // 抬起时间
} KeyTypeDef;


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

#define SCROLLBAR_WIDTH    2
#define SCROLLBAR_MARGIN   3

#define LONG_PRESS_THRESHOLD 600 // ms

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

extern Menutypedef mainMenu;
extern Menutypedef listMenu;


ItemTypedef mainMenuItems[] = 
{
  {"List", 4, NULL, &listMenu},
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

ItemTypedef listMenuItems[] = 
{
  {"List1", 5, NULL, NULL},
  {"a1", 3, NULL, NULL},
  {"wqweqw1", 7, NULL, NULL},
  {"sds1", 4, NULL, NULL},
  {"List_E1", 7, NULL, NULL},
  {"afsad1", 7, NULL, NULL},
  {"wode1", 5, NULL, NULL},
  {"List1", 5, NULL, NULL},
  {"a1", 2, NULL, NULL},
  {"wqweqw1", 7, NULL, NULL},
  {"sds1", 4, NULL, NULL},
  {"List_E1", 7, NULL, NULL},
  {"afsadf1", 7, NULL, NULL},
  {"wode1", 5, NULL, NULL}
};

Menutypedef mainMenu = {"mainMenu", mainMenuItems, NULL, sizeof(mainMenuItems) / sizeof(ItemTypedef), 0};
Menutypedef listMenu = {"listMenu", listMenuItems, &mainMenu, sizeof(listMenuItems) / sizeof(ItemTypedef), 0};
Menutypedef *currentMenu = &mainMenu;

KeyTypeDef KeyList[2] = {0};


UIElemTypedef itemX = {0};
UIElemTypedef itemY = {0};

UIElemTypedef frameWidth = {0};
UIElemTypedef frameY = {0};

UIElemTypedef screenTop = {0};
UIElemTypedef screenBottom = {0};

UIElemTypedef scrollBarY = {0};

ScreenIndexTypedef screenIndex = {0, 3};

// 0:没有按键响应 1:KEY1 2:KEY2
uint8_t keyID = 0;

// uint16_t listLen = 0;
float moveProcess_FrameY = 0.0;
float moveProcess_FrameWidth = 0.0;
float moveProcess_Screen = 0.0;
float moveProcess_ScrollBar = 0.0;

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
  
  uint32_t currentTime = HAL_GetTick();

  if (GPIO_Pin == GPIO_PIN_13)
  {
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET)
    {
      KeyList[0].val = 1;
      KeyList[0].pressTime = currentTime;
      keyID = 1;
    }
    else 
    {
      KeyList[0].val = 0;
      KeyList[0].releaseTime = currentTime;
      

      uint32_t pressDuration = KeyList[0].releaseTime - KeyList[0].pressTime;
      if (pressDuration >= LONG_PRESS_THRESHOLD)
      {
        KeyList[0].longPress = 1;  // 长按
        
      }
      else
      {
        KeyList[0].longPress = 2;// 短按
      }
    }

    

    if (!KeyList[0].val && KeyList[0].longPress)
    {
      KeyList[0].updateFlag = 1;
    }

  }
  else if (GPIO_Pin == GPIO_PIN_0)
  {
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
    {
      KeyList[1].val = 1;
      KeyList[1].pressTime = currentTime;
      keyID = 2;
    }
    else 
    {
      KeyList[1].val = 0;
      KeyList[1].releaseTime = currentTime;

      uint32_t pressDuration = KeyList[1].releaseTime - KeyList[1].pressTime;
      if (pressDuration >= LONG_PRESS_THRESHOLD)
      {
        KeyList[1].longPress = 1;  // 长按
        
      }
      else
      {
        KeyList[1].longPress = 2;// 短按
      }
      
    }

    if (!KeyList[1].val && KeyList[1].longPress)
    {
      KeyList[1].updateFlag = 1;
    }
  }
}

// 返回值： 1： 到达目标哦位置； 0： 没有到达目标位置
uint8_t UI_Move(UIElemTypedef *elem, float *moveProcess, float moveSpeed);
// uint8_t Frame_Move(UIElemTypedef *frameY, UIElemTypedef *frameWidth, float moveSpeed);
uint8_t Screen_Move(UIElemTypedef *screenTop, float moveSpeed);
void Key_Init();
void ItemIndex_UpData();
void Frame_Update();
void Screen_Update();
void ScrollBar_Update(void);
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
  frameY.targetVal = currentMenu->currentItemIndex * MENU_ITEM_HEIGHT;
  frameWidth.targetVal = currentMenu->items[currentMenu->currentItemIndex].len * 6 + 4;
  frameWidth.val = currentMenu->items[currentMenu->currentItemIndex].len * 6 + 4;
  scrollBarY.targetVal = 2;
  while (1)
  {
 
    ItemIndex_UpData();
    UI_Move(&frameY, &moveProcess_FrameY, 0.1);
    UI_Move(&frameWidth, &moveProcess_FrameWidth, 0.1);
    UI_Move(&screenTop, &moveProcess_Screen, 0.1);
    UI_Move(&scrollBarY, &moveProcess_ScrollBar, 0.1);
    // Frame_Move(&frameY, &frameWidth, 0.1);
    // Screen_Move(&screenTop, 0.1);
    UI_Show();

    // printf("%d\n", (OLED_SCREEN_HEIGHT * 4) / currentMenu->itemCount);
       
    // printf("%d, %d, %d\n", frameY.val, frameWidth.val, screenTop.val);
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
  if (KeyList[keyID - 1].updateFlag && KeyList[keyID - 1].longPress == 2)
  {
    
    KeyList[keyID - 1].updateFlag = 0;
    if (keyID == 1)
    {
      currentMenu->currentItemIndex++;
      
    }
    else if (keyID == 2)
    {
      currentMenu->currentItemIndex--;
    }

    if (currentMenu->currentItemIndex == currentMenu->itemCount)
    {
      currentMenu->currentItemIndex = 0;
    }
    else if (currentMenu->currentItemIndex == -1)
    {
      currentMenu->currentItemIndex = currentMenu->itemCount - 1;
    }
    

    Frame_Update();
    Screen_Update();
    ScrollBar_Update();
  }
}

void Frame_Update()
{
  moveProcess_FrameY = 0.0;
  moveProcess_FrameWidth = 0.0;

  frameY.lastVal = frameY.targetVal;
  frameWidth.lastVal = frameWidth.targetVal;
  
  frameY.targetVal = currentMenu->currentItemIndex * MENU_ITEM_HEIGHT;
  frameWidth.targetVal = currentMenu->items[currentMenu->currentItemIndex].len * 6 + 4;
}

void Screen_Update()
{
  if (currentMenu->currentItemIndex > screenIndex.bottomIndex)
  {
    moveProcess_Screen = 0.0;
    screenIndex.bottomIndex = currentMenu->currentItemIndex;
    screenIndex.topIndex = screenIndex.bottomIndex - 3;
  }
  else if (currentMenu->currentItemIndex < screenIndex.topIndex)
  {
    moveProcess_Screen = 0.0;
    screenIndex.topIndex = currentMenu->currentItemIndex;
    screenIndex.bottomIndex = screenIndex.topIndex + 3;
  }

  screenTop.lastVal = screenTop.targetVal;

  screenTop.targetVal = screenIndex.topIndex * MENU_ITEM_HEIGHT;
}


void ScrollBar_Update(void)
{
  moveProcess_ScrollBar = 0.0;
  
  float moveRangef = (float)((OLED_SCREEN_HEIGHT - 4) - (OLED_SCREEN_HEIGHT * 2) / currentMenu->itemCount) / (currentMenu->itemCount - 1);
  int moveRange = (int)(moveRangef + 0.5f);
  scrollBarY.lastVal = scrollBarY.val;
  if (currentMenu->currentItemIndex == currentMenu->itemCount - 1)
  {
    scrollBarY.targetVal = OLED_SCREEN_HEIGHT - 2 - (OLED_SCREEN_HEIGHT * 2) / currentMenu->itemCount;
  }
  else
  {
    scrollBarY.targetVal = (currentMenu->currentItemIndex * moveRange) + 2;
  }
}

uint8_t UI_Move(UIElemTypedef *elem, float *moveProcess, float moveSpeed)
{
  if (*moveProcess < 1.0)
  {
    *moveProcess += moveSpeed;
  }
  if (*moveProcess > 1.0)
  {
    *moveProcess = 1.0;
  } 

  float easedProcess = easeInOut(*moveProcess);

  // for (int i =0; i < sizeOfElemList; i++)
  // {
    
  // }

  elem->val = (int16_t)((1.0 - easedProcess) * elem->lastVal + easedProcess * elem->targetVal);

  if (*moveProcess == 1.0)
  {
    elem->val = elem->targetVal;
    return 1;
  }
  else
  {
    return 0;
  }
}


uint8_t Screen_Move(UIElemTypedef *screenTop, float moveSpeed)
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

// uint8_t Frame_Move(UIElemTypedef *frameY, UIElemTypedef *frameWidth, float moveSpeed)
// {

//   if (moveProcess_Frame < 1.0)
//   {
//     moveProcess_Frame += moveSpeed;
//   }
//   if (moveProcess_Frame > 1.0)
//   {
//     moveProcess_Frame = 1.0;
//   }

//   float easedProcess = easeInOut(moveProcess_Frame);

//   frameY->val = (int16_t)((1.0 - easedProcess) * frameY->lastVal + easedProcess * frameY->targetVal);
//   frameWidth->val = (int16_t)((1.0 - easedProcess) * frameWidth->lastVal + easedProcess * frameWidth->targetVal);

//   // frameY->val += (int16_t)((frameY->targetVal - frameY->val) * moveProcess);
//   // frameWidth->val += (int16_t)((frameWidth->targetVal - frameWidth->val) * moveProcess);

//   if (moveProcess_Frame == 1.0)
//   {
//     frameY->val = frameY->targetVal;
//     frameWidth->val = frameWidth->targetVal;
//     return 1;
//   }
//   else
//   {
//     return 0;
//   }

// }

void UI_Show()
{
  OLED_NewFrame();

  for (int i = -1; i < 5; i++)
  {
    OLED_PrintASCIIString(2, screenTop.targetVal - screenTop.val + (i * MENU_ITEM_HEIGHT) + 4, currentMenu->items[i + screenIndex.topIndex].str, &afont8x6, OLED_COLOR_NORMAL); 
  }

  OLED_DrawFilledRectangleWithCorners(0, frameY.val + 1 - (screenIndex.topIndex * MENU_ITEM_HEIGHT), frameWidth.val, MENU_ITEM_HEIGHT - 2, OLED_COLOR_REVERSED); 
  // Draw_ScrollBar(screenTop.targetVal, currentMenu->itemCount, 4, currentMenu->currentItemIndex);

  OLED_DrawRectangle(OLED_SCREEN_WIDTH - SCROLLBAR_WIDTH - SCROLLBAR_MARGIN - 2, 0, 6, OLED_SCREEN_HEIGHT - 1, OLED_COLOR_NORMAL);
  OLED_DrawFilledRectangle(OLED_SCREEN_WIDTH - SCROLLBAR_WIDTH - SCROLLBAR_MARGIN,
                            scrollBarY.val,
                            SCROLLBAR_WIDTH,
                            (OLED_SCREEN_HEIGHT * 2) / currentMenu->itemCount,
                            OLED_COLOR_NORMAL);
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
