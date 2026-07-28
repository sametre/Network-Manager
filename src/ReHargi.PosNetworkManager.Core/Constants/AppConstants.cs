namespace ReHargi.PosNetworkManager.Core.Constants
{
    /// <summary>
    /// Application-wide constants.
    /// </summary>
    public static class AppConstants
    {
        public const string ApplicationName = "Re Hargi – POS Network Manager";
        public const string ApplicationVersion = "1.0.0";
        public const string CompanyName = "Re Hargi";

        // Default network settings
        public const string DefaultPcIpAddress = "192.168.1.10";
        public const string DefaultPosIpAddress = "192.168.1.11";
        public const string DefaultSubnetMask = "255.255.255.0";
        public const string DefaultGateway = "192.168.1.1";
        public const string DefaultDns = "192.168.1.1";

        // Scanning defaults
        public const int DefaultPingScanConcurrency = 20;
        public const int DefaultPingTimeoutMs = 300;
        public const int DefaultScanTimeoutMs = 10000;

        // Branding colors
        public const string BrandColorRed = "#C1121F";
        public const string BrandColorDarkRed = "#8F0D16";
        public const string BrandColorLightRedBg = "#FCEBEC";
        public const string TextDark = "#202124";
        public const string TextSecondary = "#5F6368";
        public const string BorderColor = "#D6D9DE";
        public const string BackgroundMain = "#F3F4F6";
        public const string BackgroundPanel = "#FFFFFF";
        public const string ColorSuccess = "#217346";
        public const string ColorWarning = "#B26A00";
        public const string ColorError = "#B3261E";

        // Storage paths
        public const string StorageRoot = "ReHargi\\PosNetworkManager";
        public const string LogsSubfolder = "Logs";
        public const string ProfilesSubfolder = "Profiles";
        public const string BackupsSubfolder = "Backups";
    }
}
