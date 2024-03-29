/////////////////////////////////////////////
//                                         //
//    Copyright (C) 2020-2020 Julian Uy    //
//  https://sites.google.com/site/awertyb  //
//                                         //
//   See details of license at "LICENSE"   //
//                                         //
/////////////////////////////////////////////

#include "ncbind/ncbind.hpp"
#include <string.h>

class NamedPipeClient
{
	HANDLE pipehandle;
	IStream *exportstream;
	HANDLE exportthread;
public:
	NamedPipeClient() {
		pipehandle = nullptr;
		exportstream = nullptr;
		exportthread = nullptr;
	}
	~NamedPipeClient() {
		if (pipehandle)
		{
			CloseHandle(pipehandle);
			pipehandle = nullptr;
		}
		if (exportthread)
		{
			WaitForSingleObject(exportthread, INFINITE);
			CloseHandle(exportthread);
			exportthread = nullptr;
		}
		if (exportstream)
		{
			exportstream->Release();
			exportstream = nullptr;
		}
	}

	void create(ttstr path, tTVInteger openmode, tTVInteger pipemode, tTVInteger maxinstances, tTVInteger outbuffersize, tTVInteger inbuffersize, tTVInteger defaulttimeout, tTVInteger shouldinherit)
	{
		if (pipehandle)
		{
			TVPThrowExceptionMessage(TJS_W("Pipe already open"));
		}
		SECURITY_ATTRIBUTES sattr = {sizeof(SECURITY_ATTRIBUTES), nullptr, !!shouldinherit};
		pipehandle = CreateNamedPipe(path.c_str(), openmode, pipemode, maxinstances, outbuffersize, inbuffersize, defaulttimeout, &sattr);
		if (pipehandle == INVALID_HANDLE_VALUE)
		{
			ttstr mes;
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, ::GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
			mes = ttstr( (LPCWSTR)lpMsgBuf );
			::LocalFree(lpMsgBuf);
			TVPThrowExceptionMessage(TJS_W("Could not create pipe handle: %1"), mes);
		}
	}

	void open(ttstr path, tTVInteger desiredaccess, tTVInteger sharemode, tTVInteger shouldinherit, tTVInteger creationdisposition, tTVInteger flagsandattr)
	{
		if (pipehandle)
		{
			TVPThrowExceptionMessage(TJS_W("Pipe already open"));
		}
		SECURITY_ATTRIBUTES sattr = {sizeof(SECURITY_ATTRIBUTES), nullptr, !!shouldinherit};
		pipehandle = CreateFile(path.c_str(), desiredaccess, sharemode, &sattr, creationdisposition, flagsandattr, nullptr );
		if (pipehandle == INVALID_HANDLE_VALUE)
		{
			ttstr mes;
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, ::GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
			mes = ttstr( (LPCWSTR)lpMsgBuf );
			::LocalFree(lpMsgBuf);
			TVPThrowExceptionMessage(TJS_W("Could not open path: %1"), mes);
		}
	}

	void close()
	{
		if (pipehandle)
		{
			CloseHandle(pipehandle);
			pipehandle = nullptr;
		}
		if (exportthread)
		{
			CloseHandle(exportthread);
			exportthread = nullptr;
		}
		if (exportstream)
		{
			exportstream->Release();
			exportstream = nullptr;
		}
	}

	static tTVInteger waitForPath(ttstr path, tTVInteger timeout)
	{
		BOOL retval = FALSE;
		LARGE_INTEGER count_to_sec;
		LARGE_INTEGER ms_start;
		LARGE_INTEGER ms_end;
		LARGE_INTEGER ms_last;
		QueryPerformanceFrequency(&count_to_sec);
		QueryPerformanceCounter(&ms_start);
		ms_end.QuadPart = ms_start.QuadPart + (timeout * (count_to_sec.QuadPart / 1000));
		ms_last.QuadPart = ms_start.QuadPart;

		while (!retval && (ms_end.QuadPart > ms_last.QuadPart))
		{
			retval = WaitNamedPipe(path.c_str(), (ms_end.QuadPart - ms_last.QuadPart));
			QueryPerformanceCounter(&ms_last);
		}
		return retval;
	}

	void waitForConnection()
	{
		if (!pipehandle)
		{
			TVPThrowExceptionMessage(TJS_W("Pipe isn't opened"));
		}
		DWORD err = ::ConnectNamedPipe(pipehandle, NULL) ? 0 : ::GetLastError();
		if (err)
		{
			ttstr mes;
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
			mes = ttstr( (LPCWSTR)lpMsgBuf );
			::LocalFree(lpMsgBuf);
			TVPThrowExceptionMessage(TJS_W("Could not open path: %1"), mes);
		}
	}

	void setState(tTJSVariant mode, tTJSVariant maxcollectioncount, tTJSVariant collectdatatimeout)
	{
		if (!pipehandle)
		{
			TVPThrowExceptionMessage(TJS_W("Pipe isn't opened"));
		}
		DWORD mode_dw = mode.AsInteger();
		DWORD maxcollectioncount_dw = maxcollectioncount.AsInteger();
		DWORD collectdatatimeout_dw = collectdatatimeout.AsInteger();
		SetNamedPipeHandleState(pipehandle, (mode.Type() == tvtVoid) ? NULL : &mode_dw, (maxcollectioncount.Type() == tvtVoid) ? NULL : &maxcollectioncount_dw, (collectdatatimeout.Type() == tvtVoid) ? NULL : &collectdatatimeout_dw);
	}

	tTJSVariant read()
	{
		DWORD byteshandled = 0;
		if (!pipehandle)
		{
			TVPThrowExceptionMessage(TJS_W("Pipe isn't opened"));
		}
		DWORD bytesavail = 0;
		DWORD bytesleftthismessage = 0;
		BOOL res1 = PeekNamedPipe(pipehandle, NULL, 0, NULL, &bytesavail, &bytesleftthismessage);
		if (!res1)
		{
			ttstr mes;
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, ::GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
			mes = ttstr( (LPCWSTR)lpMsgBuf );
			::LocalFree(lpMsgBuf);
			TVPThrowExceptionMessage(TJS_W("Could not check pipe handle state: %1"), mes);
		}
		if (bytesavail == 0)
		{
			return tTJSVariant();
		}
		DWORD totalbytes = bytesavail;
		if (bytesleftthismessage != 0)
		{
			totalbytes = bytesleftthismessage;
		}
		BYTE *data = (BYTE *)malloc(totalbytes);
		if (!data)
		{
			TVPThrowExceptionMessage(TJS_W("Could not allocate memory"));
		}
		BYTE *data_curpos = data;
		DWORD bytesleft = totalbytes;
		BOOL res2 = FALSE; 
		do
		{
			res2 = ReadFile(pipehandle, data, bytesleft, &byteshandled, NULL);
			if (!res2 && ::GetLastError() == ERROR_MORE_DATA && bytesleft != byteshandled)
			{
				bytesleft -= byteshandled;
				data_curpos += byteshandled;
				continue;
			}
			else
			{
				break;
			}
		} while (1);
		
		if (!res2)
		{
			ttstr mes;
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, ::GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
			mes = ttstr( (LPCWSTR)lpMsgBuf );
			::LocalFree(lpMsgBuf);
			free(data);
			TVPThrowExceptionMessage(TJS_W("Could not read from pipe handle: %1"), mes);
		}
		tTJSVariant v((const tjs_uint8 *)&data[0], totalbytes);
		free(data);
		return v;
	}

	tTVInteger write(tTJSVariant buffer_octet)
	{
		DWORD byteshandled = 0;
		if (!pipehandle)
		{
			TVPThrowExceptionMessage(TJS_W("Pipe isn't opened"));
		}
		BOOL res = WriteFile(pipehandle, buffer_octet.AsOctetNoAddRef()->GetData(), buffer_octet.AsOctetNoAddRef()->GetLength(), &byteshandled, NULL);
		if (!res)
		{
			ttstr mes;
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, ::GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
			mes = ttstr( (LPCWSTR)lpMsgBuf );
			::LocalFree(lpMsgBuf);
			TVPThrowExceptionMessage(TJS_W("Could not write to pipe handle: %1"), mes);
		}
		return byteshandled;
	}

	void exportProcess()
	{
		BYTE buffer[1024*16];
		DWORD size, byteshandled;
		DWORD err = ::ConnectNamedPipe(pipehandle, NULL) ? 0 : ::GetLastError();
		if (err != ERROR_PIPE_CONNECTED && err != 0)
		{
			return;
		}
		while (exportstream->Read(buffer, sizeof buffer, &size) == S_OK && size > 0 && WriteFile(pipehandle, buffer, size, &byteshandled, NULL));
		if (pipehandle)
		{
			CloseHandle(pipehandle);
			pipehandle = nullptr;
		}
	}

	static DWORD WINAPI exportThread(LPVOID this_)
	{
		if (this_ == nullptr)
		{
			return (DWORD)-1;
		}
		((NamedPipeClient *)this_)->exportProcess();
		return 0;
	}

	void exportFileAsync(ttstr path)
	{
		if (exportthread)
		{
			TVPThrowExceptionMessage(TJS_W("File currently being exported"));
		}
		if (!pipehandle)
		{
			TVPThrowExceptionMessage(TJS_W("Pipe isn't opened"));
		}
		exportstream = TVPCreateIStream(path, TJS_BS_READ);
		if (!exportstream)
		{
			TVPThrowExceptionMessage((ttstr(TJS_W("Cannot open file for export: ")) + path).c_str());
		}
		DWORD threadid;
		exportthread = CreateThread(NULL, 0, exportThread, (LPVOID) this, 0, &threadid);
		if (!exportthread)
		{
			ttstr mes;
			LPVOID lpMsgBuf;
			::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, ::GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
			mes = ttstr( (LPCWSTR)lpMsgBuf );
			::LocalFree(lpMsgBuf);
			exportstream->Release();
			exportstream = nullptr;
			TVPThrowExceptionMessage(TJS_W("Could not create export thread: %1"), mes);
		}
	}

	void exportFileWait()
	{
		if (exportthread)
		{
			if (pipehandle)
			{
				WaitForSingleObject(exportthread, INFINITE);
			}
			CloseHandle(exportthread);
			exportthread = nullptr;
		}
		if (exportstream)
		{
			exportstream->Release();
			exportstream = nullptr;
		}
	}

	void exportFileKill()
	{
		HANDLE savehandle = pipehandle;
		pipehandle = nullptr;
		if (exportthread)
		{
			if (pipehandle)
			{
				WaitForSingleObject(exportthread, INFINITE);
			}
			CloseHandle(exportthread);
			exportthread = nullptr;
		}
		if (exportstream)
		{
			exportstream->Release();
			exportstream = nullptr;
		}
		pipehandle = savehandle;
	}
};

NCB_REGISTER_CLASS(NamedPipeClient)
{
	Constructor();

	NCB_METHOD(create);
	NCB_METHOD(open);
	NCB_METHOD(close);
	NCB_METHOD(waitForPath);
	NCB_METHOD(waitForConnection);
	NCB_METHOD(setState);
	NCB_METHOD(read);
	NCB_METHOD(write);
	NCB_METHOD(exportFileAsync);
	NCB_METHOD(exportFileWait);
	NCB_METHOD(exportFileKill);
};
