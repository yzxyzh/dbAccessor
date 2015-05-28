#pragma once

#define GUI_BEGIN try{

#define GUI_END }catch(...){ShowError("error occured! please retry!");}