#ifndef HTMLVIEWER_H
#define HTMLVIEWER_H

void DestroyHTMLViewer(HWND hWnd);
void UnRegisterHTMLViewer();
void ResizeHTMLViewer(HWND hWnd, long width, long height);
void HTMLViewerMessage(MSG *msg);
void CopyHTMLViewerSelectionToClipBoard(HWND hWnd);
bool RegisterHTMLViewer();
void InitialiseHtmlViewer(HWND hWnd);
void EnableHtmlViewer(HWND hWnd, short enable);
void LoadURLIntoHTMLViewer(HWND hWnd, char* URL);
void GoBackHTMLViewer(HWND hWnd);
void GoForwardHTMLViewer(HWND hWnd);
void CopyHTMLViewerSelectionToClipBoard(HWND hWnd);
void SetHTMLViewerFocus(HWND child, HWND parent);
void RunHTMLViewerSelection(HWND hWnd);

#endif // define HTMLVIEWER_H