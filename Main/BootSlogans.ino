// Array of Bitcoin slogans
String bitcoinSlogans[] = {
  "Bitcoin is gold for nerds",
  "In Bitcoin we trust.",
  "Be your own bank.",
  "HODL"
};


// Function to get a random Bitcoin slogan from the array
String getRandomBootSlogan() {
  int numOfSlogans = sizeof(bitcoinSlogans) / sizeof(bitcoinSlogans[0]); // Calculate the number of slogans in the array

  int randomIndex = random(0, numOfSlogans); // Generate a random index between 0 and numOfSlogans-1
  return bitcoinSlogans[randomIndex]; 
}