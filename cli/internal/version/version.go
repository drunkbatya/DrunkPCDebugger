package version

import "fmt"

// compile-time variables
var gitCommit string
var gitBranch string
var gitBranchNum string
var gitOrigin string
var gitDirty string
var buildDate string
var buildTime string
var version string

type BuildInfoStruct struct {
	GitCommit    string
	GitBranch    string
	GitBranchNum string
	GitOrigin    string
	GitDirty     string
	BuildDate    string
	BuildTime    string
	Version      string
}

var BuildInfo = &BuildInfoStruct{
	GitCommit:    gitCommit,
	GitBranch:    gitBranch,
	GitBranchNum: gitBranchNum,
	GitOrigin:    gitOrigin,
	GitDirty:     gitDirty,
	BuildDate:    buildDate,
	BuildTime:    buildTime,
	Version:      version,
}

func GetPrintableString() string {
	return fmt.Sprintf("%s commit [%s] branch [%s] dirty [%s] built at [%s %s] origin [%s]",
		BuildInfo.Version,
		BuildInfo.GitCommit,
		BuildInfo.GitBranch,
		BuildInfo.GitDirty,
		BuildInfo.BuildDate,
		BuildInfo.BuildTime,
		BuildInfo.GitOrigin,
	)
}
