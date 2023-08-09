String getLatestVersion() {
  return getEndpointData(checkUpdateHost, "/");
}

void showUpdateAvailable() {
  // TODO: display "update available" somewhere
}
