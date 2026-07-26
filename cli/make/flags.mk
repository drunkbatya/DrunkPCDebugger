LDFLAGS := \
	-X $(TARGET)/internal/version.gitCommit=$(GIT_COMMIT) \
	-X $(TARGET)/internal/version.gitBranch=$(GIT_BRANCH) \
	-X $(TARGET)/internal/version.gitBranchNum=$(GIT_BRANCH_NUM) \
	-X $(TARGET)/internal/version.buildDate=$(BUILD_DATE) \
	-X $(TARGET)/internal/version.buildTime=$(BUILD_TIME) \
	-X $(TARGET)/internal/version.version=$(BUILD_VERSION)
