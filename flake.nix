{
  description = "ZotBins Core";

  inputs = {
    esp-dev = {
      url = "github:mirrexagon/nixpkgs-esp-dev";
    };
  };

  outputs = { self, esp-dev }:
    let
      system = "x86_64-linux";
    in
    {
      devShells."${system}" = {
        default = esp-dev.devShells."${system}".esp32-idf;
      };
    };
}
