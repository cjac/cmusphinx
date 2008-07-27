:: Ensures that full tree is there; note that the compiler dies anyway if no grammar defined
@ECHO OFF
IF NOT EXIST Resources MKDIR Resources
IF NOT EXIST Resources\DecoderConfig MKDIR Resources\DecoderConfig
IF NOT EXIST Resources\DecoderConfig\Dictionary MKDIR Resources\DecoderConfig\Dictionary
IF NOT EXIST Resources\DecoderConfig\LanguageModel MKDIR Resources\DecoderConfig\LanguageModel
IF NOT EXIST Resources\DecoderConfig\LanguageModel\temp MKDIR Resources\DecoderConfig\LanguageModel\TEMP
IF NOT EXIST Resources\Grammar MKDIR Resources\Grammar
IF NOT EXIST Resources\Grammar\GRAMMAR MKDIR Resources\Grammar\GRAMMAR
::
