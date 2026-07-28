using System;
using System.Security.Principal;
using System.Windows.Forms;

namespace ReHargi.PosNetworkManager
{
    static class Program
    {
        /// <summary>
        /// Application entry point.
        /// </summary>
        [STAThread]
        static void Main()
        {
            // Check for administrator privileges
            if (!IsRunningAsAdministrator())
            {
                MessageBox.Show(
                    "Bu uygulama yönetici yetkisi ile çalıştırılmalıdır.\n\nThe application requires administrator privileges.",
                    "Re Hargi – POS Network Manager",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Warning);
                return;
            }

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Forms.MainForm());
        }

        /// <summary>
        /// Checks whether the application is running with administrator privileges.
        /// </summary>
        private static bool IsRunningAsAdministrator()
        {
            try
            {
                var identity = WindowsIdentity.GetCurrent();
                var principal = new WindowsPrincipal(identity);
                return principal.IsInRole(WindowsBuiltInRole.Administrator);
            }
            catch
            {
                return false;
            }
        }
    }
}
