// #include "UI_Move.h"

// void Key_Init()
// {
//   memset(KeyList, 0, sizeof(KeyList));
// }



// void ItemIndex_UpData()
// {
//   if (KeyList[keyID - 1].updateFlag && KeyList[keyID - 1].val && keyID)
//   {
    
//     KeyList[keyID - 1].updateFlag = 0;
//     if (keyID == 1)
//     {
//       itemIndex++;
      
//     }
//     else if (keyID == 2)
//     {
//       itemIndex--;
//     }

//     if (itemIndex == listLen)
//     {
//       itemIndex = 0;
//     }
//     else if (itemIndex == -1)
//     {
//       itemIndex = listLen - 1;
//     }
    

//     Frame_UpData();
//     Screen_UpData();
//   }
// }

// void Frame_UpData()
// {
//   moveProcess_Frame = 0.0;
//   frameY.lastVal = frameY.targetVal;
//   frameWidth.lastVal = frameWidth.targetVal;
  
//   frameY.targetVal = itemIndex * MENU_ITEM_HEIGHT;
//   frameWidth.targetVal = itemList[itemIndex].len * 6 + 4;
// }

// void Screen_UpData()
// {
//   if (itemIndex >= screenIndex.bottomIndex)
//   {
//     moveProcess_Screen = 0.0;
//     screenIndex.bottomIndex = itemIndex;
//     screenIndex.topIndex = screenIndex.bottomIndex - 3;
//   }
//   else if (itemIndex <= screenIndex.topIndex)
//   {
//     moveProcess_Screen = 0.0;
//     screenIndex.topIndex = itemIndex;
//     screenIndex.bottomIndex = screenIndex.topIndex + 3;
//   }

//   screenTop.lastVal = screenTop.targetVal;

//   screenTop.targetVal = screenIndex.topIndex * MENU_ITEM_HEIGHT;

// }

// uint8_t Screen_Move(UIContextTypedef *screenTop, float moveSpeed)
// {
//   if (moveProcess_Screen < 1.0)
//   {
//     moveProcess_Screen += moveSpeed;
//   }
//   if (moveProcess_Screen > 1.0)
//   {
//     moveProcess_Screen = 1.0;
//   }

//   float easedProcess = easeInOut(moveProcess_Screen);

//   screenTop->val = (int16_t)((1.0 - easedProcess) * screenTop->lastVal + easedProcess * screenTop->targetVal);

//   if (moveProcess_Screen == 1.0)
//   {
//     screenTop->val = screenTop->targetVal;
//     return 1;
//   }
//   else
//   {
//     return 0;
//   }
// }

// uint8_t Frame_Move(UIContextTypedef *frameY, UIContextTypedef *frameWidth, float moveSpeed)
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

// void UI_Show()
// {
//   OLED_NewFrame();
//   printf("%d, %d\n", screenTop.val, frameY.val);
//   for (int i = -1; i < 5; i++)
//   {
//     OLED_PrintASCIIString(2, screenTop.targetVal - screenTop.val + (i * MENU_ITEM_HEIGHT) + 4, itemList[i + screenIndex.topIndex].str, &afont8x6, OLED_COLOR_NORMAL); 
//   }
//   OLED_DrawFilledRectangle(0, frameY.val + 1 - (screenIndex.topIndex * MENU_ITEM_HEIGHT), frameWidth.val, MENU_ITEM_HEIGHT - 2, OLED_COLOR_REVERSED); 
//   OLED_ShowFrame();
// }

// float easeInOut(float t) 
// {
//   if (t < 0.5)
//     return 2 * t * t;
//   else
//     return -1 + (4 - 2 * t) * t;
// }