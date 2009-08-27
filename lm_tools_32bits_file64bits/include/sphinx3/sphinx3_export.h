#ifndef __S3DECODER_EXPORT_H__
#define __S3DECODER_EXPORT_H__

/* Win32/WinCE DLL gunk */
#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(CYGWIN)
#ifdef S3DECODER_EXPORTS
#define S3DECODER_EXPORT __declspec(dllexport)
#else
#define S3DECODER_EXPORT __declspec(dllimport)
#endif
#else /* !_WIN32 */
#define S3DECODER_EXPORT
#endif

#endif /* __S3DECODER_EXPORT_H__ */
