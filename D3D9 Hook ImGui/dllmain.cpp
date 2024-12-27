#define _CRT_SECURE_NO_WARNINGS

#include "Hook.h"
#include "d3d9.h"
#include "d3dx9.h"

#pragma comment ( lib, "d3d9.lib")
#pragma comment ( lib, "d3dx9.lib")

bool hkWall = true;
bool hkCham = false;

uintptr_t lpJmpReset = -1;
uintptr_t lpJmpDrawIndexedPrimitive = -1;
uintptr_t lpJmpCreateQuery = -1;

DWORD ResetJMP = NULL;
DWORD DIPJMP = NULL;
DWORD CQJMP = NULL;

DWORD AddrCQuery = 0x10CCD39A;
DWORD AddrReset = 0x110AAFB8;
DWORD AddrDrawPrimitive = 0x110BB010;

static int size_RS = 0x5;
static int size_CQ = 0x5;
static int size_DIP = 0xB;

static LPDIRECT3DDEVICE9 pDevice;

LPDIRECT3DTEXTURE9 RED = NULL;
LPDIRECT3DTEXTURE9 GREEN = NULL;
LPDIRECT3DTEXTURE9 BLUE = NULL;
LPDIRECT3DTEXTURE9 YELLOW = NULL;
LPDIRECT3DTEXTURE9 PINK = NULL;
LPDIRECT3DTEXTURE9 WHITE = NULL;
LPDIRECT3DTEXTURE9 BLACK = NULL;
LPDIRECT3DTEXTURE9 PURPLE = NULL;

#define WHITE1          D3DCOLOR_ARGB(255, 255, 255, 255)
#define RED1            D3DCOLOR_ARGB(255, 255, 000, 000)
#define GREEN1          D3DCOLOR_ARGB(255, 000, 255, 000)
#define BLUE1           D3DCOLOR_ARGB(255, 000, 000, 255)
#define BLACK1          D3DCOLOR_ARGB(150, 000, 000, 000)
#define nBLACK1         D3DCOLOR_ARGB(100, 000, 000, 205)
#define PURPLE1         D3DCOLOR_ARGB(255, 125, 000, 255)
#define GREY            D3DCOLOR_ARGB(255, 128, 128, 128)
#define YELLOW1         D3DCOLOR_ARGB(255, 255, 255, 000)
#define ORANGE1         D3DCOLOR_ARGB(255, 255, 125, 000)
#define PINK1           D3DCOLOR_ARGB(255, 255, 050, 255)
#define CYAN1           D3DCOLOR_ARGB(255, 000, 255, 255)
#define WHITE21         D3DCOLOR_ARGB(100, 255, 255, 255)
#define RED21           D3DCOLOR_ARGB(100, 255, 000, 000)
#define GREEN21         D3DCOLOR_ARGB(100, 000, 255, 000)
#define BLUE21          D3DCOLOR_ARGB(100, 000, 000, 255)
#define BLACK21         D3DCOLOR_ARGB(100, 000, 000, 000)
#define PURPLE21        D3DCOLOR_ARGB(100, 125, 000, 255)
#define GREY21          D3DCOLOR_ARGB(100, 128, 128, 128)
#define YELLOE21        D3DCOLOR_ARGB(100, 255, 255, 000)
#define ORANGE21        D3DCOLOR_ARGB(100, 255, 125, 000)
#define PINK21          D3DCOLOR_ARGB(100, 255, 250, 255)
#define MCOLOR_CURRENT  D3DCOLOR_ARGB(255, 255, 255, 255)
#define MCOLOR_TEXT     D3DCOLOR_ARGB(255, 255, 125, 255)
#define MCOLOR_ACTIVE   D3DCOLOR_ARGB(255, 000, 255, 000)
#define MENUCOLOR_TITLE D3DCOLOR_ARGB(255, 255, 000, 000)
#define MCOLOR_FOLDER   D3DCOLOR_ARGB(255, 255, 255, 000)
#define MCOLOR_INACTIVE D3DCOLOR_ARGB(255, 255, 000, 000)

HRESULT GenerateTexture(LPDIRECT3DDEVICE9 pDevice, LPDIRECT3DTEXTURE9& texture, D3DCOLOR color)
{
	static LPDIRECT3DTEXTURE9 pTx = NULL;
	static D3DLOCKED_RECT d3dlr;
	if (pDevice->CreateTexture(8, 8, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL) == S_OK)
	{
		if (pDevice->CreateTexture(8, 8, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pTx, NULL) == S_OK)
		{
			if (pTx->LockRect(0, &d3dlr, 0, D3DLOCK_DONOTWAIT | D3DLOCK_NOSYSLOCK) == S_OK)
			{
				for (UINT xy = 0; xy < 8 * 8; xy++)((PDWORD)d3dlr.pBits)[xy] = color;
				pTx->UnlockRect(0);
				pDevice->UpdateTexture(pTx, texture);
				pTx->Release();
			}
		}
	}
	return S_OK;
}

VOID WINAPI Reset(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*) {
	if (RED) {
		RED->Release();
		RED = NULL;
	}
	if (GREEN) {
		GREEN->Release();
		GREEN = NULL;
	}
	if (BLUE) {
		BLUE->Release();
		BLUE = NULL;
	}
	if (PURPLE) {
		PURPLE->Release();
		PURPLE = NULL;
	}

}



VOID WINAPI DrawIndexedPrimitive(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE pType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertex, UINT StartIndex, UINT PrimCount) {
	if (RED == NULL)
		GenerateTexture(pDevice, RED, RED1);
	if (GREEN == NULL)
		GenerateTexture(pDevice, GREEN, GREEN1);
	if (BLUE == NULL)
		GenerateTexture(pDevice, BLUE, BLUE1);
	if (BLUE == NULL)
		GenerateTexture(pDevice, PURPLE, PURPLE21);

	LPDIRECT3DVERTEXBUFFER9 pStream;
	UINT Offset, Stride;

	if (SUCCEEDED(pDevice->GetStreamSource(NULL, &pStream, &Offset, &Stride) == D3D_OK))
		pStream->Release();

	if ((Stride == 52 || Stride == 32))
	{
		pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		pDevice->DrawIndexedPrimitive(pType, BaseVertexIndex, MinVertexIndex, NumVertex, StartIndex, PrimCount);
		pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	}
}

_declspec (naked)
VOID WINAPI Reset_Function()
{
	_asm
	{
		pushad;
		push esp;
		sub esp, 0x400;
		call Reset;
		add esp, 0x400;
		pop esp;
		popad;
		push edi;
		push eax;
		call dword ptr[ecx + 0x40];
		jmp lpJmpReset;
	}
}

VOID __declspec (naked) WINAPI DrawIndexedPrimitive_Function()
{
	_asm
	{
		push ebx;
		push ecx;
		push 0x4;
		push eax;
		push ebp;
		mov ebp, esp;
		pushad;
		pushfd;
		push esp;
		sub esp, 0x400;
		push[ebp + 0x1C];
		push[ebp + 0x18];
		push[ebp + 0x14];
		push[ebp + 0x10];
		push[ebp + 0xC];
		push[ebp + 0x8];
		push[ebp + 0x4];
		call DrawIndexedPrimitive;
		add esp, 0x400;
		pop esp;
		popfd;
		popad;
		pop ebp;
		call dword ptr[edx + 0x148];
		jmp DIPJMP;
	}
}

_declspec (naked)
VOID WINAPI hkCreateQury1()
{
	_asm
	{
		mov edx, dword ptr[ecx];
		push 0x8;
		push ecx;
		jmp CQJMP;
	}
}

void MakeJMP(BYTE* pAddress, DWORD dwJumpTo, DWORD dwLen)
{
	DWORD dwOldProtect, dwBkup, dwRelAddr;
	VirtualProtect(pAddress, dwLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	dwRelAddr = (DWORD)(dwJumpTo - (DWORD)pAddress) - 5;
	*pAddress = 0xE9;
	*((DWORD*)(pAddress + 0x1)) = dwRelAddr;
	for (DWORD x = 0x5; x < dwLen; x++) *(pAddress + x) = 0x90;
	VirtualProtect(pAddress, dwLen, dwOldProtect, &dwBkup);
	return;
}

VOID WriteToMemery(DWORD address, char* value, int size)
{
	DWORD OldProtection;
	VirtualProtectEx(GetCurrentProcess(), (LPVOID)address, size, PAGE_EXECUTE_READWRITE, &OldProtection);
	memcpy((LPVOID)address, value, size);
	VirtualProtectEx(GetCurrentProcess(), (LPVOID)address, size, OldProtection, NULL);
}

DWORD WINAPI Attach(LPVOID) {
	while (GetModuleHandleA("MAT.exe") == NULL) {
		Sleep(500);
	}

	ResetJMP = AddrReset + size_RS;
	MakeJMP((PBYTE)AddrReset, (DWORD)Reset_Function, size_RS);

	DIPJMP = AddrDrawPrimitive + size_DIP;
	MakeJMP((PBYTE)AddrDrawPrimitive, (DWORD)DrawIndexedPrimitive_Function, size_DIP);

	CQJMP = AddrCQuery + size_CQ;
	MakeJMP((PBYTE)AddrCQuery, (DWORD)hkCreateQury1, size_CQ);

	return FALSE;
}

BOOL WINAPI DllMain(const HINSTANCE hinstDLL, const DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
		Hook::hDDLModule = hinstDLL;
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Hook::HookDirectX, nullptr, 0, nullptr);
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Attach, nullptr, 0, nullptr);
	}

	if (fdwReason == DLL_PROCESS_DETACH)
		Hook::UnHookDirectX();

	return TRUE;
}