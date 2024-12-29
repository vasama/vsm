#!/usr/bin/pwsh

param(
	[string]$GitHubSourceUser,
	[string]$GitHubSourceRepository,
	[string]$GitHubSourceRef,

	[string]$GitHubIndexUser,
	[string]$GitHubIndexRepository,
	[string]$GitHubIndexEvent,
	[string]$GitHubIndexToken,

	[string]$PackageBase=$null
)

if (!($GitHubSourceRef -match '^tags/release/([^/]+)/([^/]+)$')) {
	throw 'The specified Git ref is not a valid release tag.'
}

$PackagePath = $Matches[1]
$PackageVersion = $Matches[2]

$PackageName = $PackagePath
if ($PackageBase) {
	$PackageName = "$PackageBase-$PackagePath"
}

$ClientPayload = @{
	package_path=$PackagePath;
	package_name=$PackageName;
	package_version=$PackageVersion;
	github_user=$GitHubSourceUser;
	github_repository=$GitHubSourceRepository;
	github_ref=$GitHubSourceRef;
}

$RequestUrl = "https://api.github.com/repos/$GitHubIndexUser/$GitHubIndexRepository/dispatches"

$RequestHeaders = @{}
$RequestHeaders['Accept'] = 'application/vnd.github+json'
$RequestHeaders['Authorization'] = "Bearer $GitHubIndexToken"
$RequestHeaders['X-GitHub-Api-Version'] = '2022-11-28'

$RequestContent = ConvertTo-Json @{
	event_type=$GitHubIndexEvent;
	client_payload=$ClientPayload;
}

Invoke-WebRequest -Uri $RequestUrl -Method Post -Headers $RequestHeaders -Body $RequestContent
