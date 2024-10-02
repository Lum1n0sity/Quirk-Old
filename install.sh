if [ "$(id -u)" -ne 0 ]; then
  echo "Please run this script with sudo: sudo ./install.sh"
  exit 1
fi

# Step 1: Run the llvm.sh script
echo "Running llvm.sh..."
sudo ./llvm.sh
if [ $? -ne 0 ]; then
  echo "Error running llvm.sh. Exiting."
  exit 1
fi

# Step 2: Install cmake
echo "Installing cmake..."
sudo apt update && sudo apt install -y cmake
if [ $? -ne 0 ]; then
  echo "Error installing cmake. Exiting."
  exit 1
fi

# Step 3: Install required dependencies
echo "Installing required dependencies..."
sudo apt install -y libedit-dev libzstd-dev libcurl4-openssl-dev
if [ $? -ne 0 ]; then
  echo "Error installing dependencies. Exiting."
  exit 1
fi

echo "Installation completed successfully."
