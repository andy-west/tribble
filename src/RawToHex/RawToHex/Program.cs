using System.Text;

namespace RawToHex;

public class Program
{
    public static void Main()
    {
        ConvertRawToHex("tos_tribble_angry.raw", "tos_tribble_angry.hex");
        ConvertRawToHex("tos_tribble_coos.raw", "tos_tribble_coos.hex");
    }

    public static void ConvertRawToHex(string rawFilePath, string hexFilePath)
    {
        if (!File.Exists(rawFilePath))
        {
            throw new FileNotFoundException("The specified raw file does not exist.");
        }

        using var rawStream = new FileStream(rawFilePath, FileMode.Open, FileAccess.Read);
        using var hexStream = new StreamWriter(hexFilePath, false, Encoding.ASCII);

        var buffer = new byte[10];
        int bytesRead;
        int totalBytes = 0;

        while ((bytesRead = rawStream.Read(buffer, 0, buffer.Length)) > 0)
        {
            var line = new StringBuilder("    ");

            for (var i = 0; i < bytesRead; i++)
            {
                if (i > 0) line.Append(",");
                line.AppendFormat("0x{0:X2}", buffer[i]);
                totalBytes++;
            }

            line.Append(",");

            hexStream.WriteLine(line);
        }

        Console.WriteLine($"File bytes: {totalBytes}");
    }
}
