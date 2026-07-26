package version

import "fmt"

// compile-time variables
var gitCommit string
var gitBranch string
var gitBranchNum string
var buildDate string
var buildTime string
var version string

type BuildInfoStruct struct {
	GitCommit    string
	GitBranch    string
	GitBranchNum string
	BuildDate    string
	BuildTime    string
	Version      string
}

var BuildInfo = &BuildInfoStruct{
	GitCommit:    gitCommit,
	GitBranch:    gitBranch,
	GitBranchNum: gitBranchNum,
	BuildDate:    buildDate,
	BuildTime:    buildTime,
	Version:      version,
}

func GetPrintableString() string {
	return fmt.Sprintf("%s commit [%s] branch [%s] built at [%s %s]",
		BuildInfo.Version,
		BuildInfo.GitCommit,
		BuildInfo.GitBranch,
		BuildInfo.BuildDate,
		BuildInfo.BuildTime,
	)
}
