package logging

import (
	"os"
	"strings"
	"time"

	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
)

func NewAppLogger(level string) (*zap.Logger, error) {
	encoderCfg := zapcore.EncoderConfig{
		TimeKey:          "time",
		LevelKey:         "level",
		CallerKey:        "caller",
		MessageKey:       "msg",
		ConsoleSeparator: " ",
		EncodeTime:       encodeBracketTime,
		EncodeLevel:      encodeUpperLevel,
		EncodeCaller:     encodeCaller,
	}

	core := zapcore.NewCore(
		zapcore.NewConsoleEncoder(encoderCfg),
		zapcore.Lock(os.Stdout),
		parseAtomicLevel(level),
	)
	return zap.New(core, zap.AddCaller()), nil
}

func ParseLevel(level string) zapcore.Level {
	return parseLevel(level)
}

func parseAtomicLevel(level string) zap.AtomicLevel {
	return zap.NewAtomicLevelAt(parseLevel(level))
}

func parseLevel(level string) zapcore.Level {
	switch strings.ToLower(level) {
	case "debug":
		return zap.DebugLevel
	case "warn":
		return zap.WarnLevel
	case "error":
		return zap.ErrorLevel
	default:
		return zap.InfoLevel
	}
}

func encodeBracketTime(ts time.Time, enc zapcore.PrimitiveArrayEncoder) {
	enc.AppendString("[" + ts.Format("2006-01-02 15:04:05") + "]")
}

func encodeUpperLevel(level zapcore.Level, enc zapcore.PrimitiveArrayEncoder) {
	enc.AppendString(strings.ToUpper(level.String()))
}

func encodeCaller(caller zapcore.EntryCaller, enc zapcore.PrimitiveArrayEncoder) {
	enc.AppendString(caller.TrimmedPath() + ":")
}
