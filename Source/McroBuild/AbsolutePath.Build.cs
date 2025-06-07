/** @noop License Comment
 *  @file
 *  @copyright
 *  This Source Code is subject to the terms of the Mozilla Public License, v2.0.
 *  If a copy of the MPL was not distributed with this file You can obtain one at
 *  https://mozilla.org/MPL/2.0/
 *
 *  @author David Mórász
 *  @date 2025
 */

/**
 * @file
 * An attempt to bring the expressiveness of NUKE's AbsolutePath into Unreal module rules and targets.
 */

using System;
using System.Linq;
using System.IO;
using System.Collections.Generic;
using EpicGames.Core;

namespace McroBuild;

/// <summary>
/// A simplified copy of NUKE's own AbsolutePath class
/// https://github.com/nuke-build/nuke/blob/develop/source/Nuke.Utilities/IO/AbsolutePath.cs
/// </summary>
public class AbsolutePath: IFormattable
{
	private readonly string _path;

	private AbsolutePath(string path)
	{
		_path = PathUtils.NormalizePath(path);
	}

	public static AbsolutePath Create(string path)
	{
		return new AbsolutePath(path);
	}
	
	public static implicit operator AbsolutePath(string path)
	{
		if (path is null)
			return null;
		if (!PathUtils.HasPathRoot(path)) throw new Exception($"Path '{path}' must be rooted");
		return new AbsolutePath(path);
	}

	public static implicit operator string(AbsolutePath path)
	{
		return path?.ToString();
	}
	
	public string Name => Path.GetFileName(_path);
	
	public string NameWithoutExtension => Path.GetFileNameWithoutExtension(_path);

	public string Extension => Path.GetExtension(_path);

	public AbsolutePath Parent =>
		!PathUtils.IsWinRoot(_path.TrimEnd(PathUtils.WinSeparator)) && !PathUtils.IsUncRoot(_path) && !PathUtils.IsUnixRoot(_path)
			? this / ".."
			: null;
	
	public static AbsolutePath operator / (AbsolutePath left, Range range)
	{
		return left.Parent;
	}

	public static AbsolutePath operator / (AbsolutePath left, string right)
	{
		return new AbsolutePath(PathUtils.Combine(left!, right));
	}

	public static AbsolutePath operator + (AbsolutePath left, string right)
	{
		return new AbsolutePath(left.ToString() + right);
	}
	
	protected bool Equals(AbsolutePath other)
	{
		var stringComparison = PathUtils.HasWinRoot(_path) ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal;
		return string.Equals(_path, other._path, stringComparison);
	}

	public static bool operator == (AbsolutePath a, AbsolutePath b)
	{
		return a!.Equals(b);
	}

	public static bool operator !=(AbsolutePath a, AbsolutePath b)
	{
		return !a!.Equals(b);
	}

	public override bool Equals(object obj)
	{
		if (ReferenceEquals(objA: null, obj))
			return false;
		if (ReferenceEquals(this, obj))
			return true;
		if (obj.GetType() != GetType())
			return false;
		return Equals((AbsolutePath) obj);
	}

	public override int GetHashCode()
	{
		return _path?.GetHashCode() ?? 0;
	}

	public override string ToString()
	{
		return ((IFormattable)this).ToString(format: null, formatProvider: null);
	}

	public const string DoubleQuote = "dq";
	public const string SingleQuote = "sq";
	public const string NoQuotes = "nq";
	
	string IFormattable.ToString(string format, IFormatProvider formatProvider)
	{
		var path = _path.WithUnixSeparator();
		return format switch
		{
			DoubleQuote => $"\"{path}\"",
			SingleQuote => $"'{path}'",
			null or NoQuotes => path,
			_ => throw new ArgumentException($"Format '{format}' is not recognized")
		};
	}
}

public static partial class AbsolutePathExtensions
{
	public static AbsolutePath GetRoot(this AbsolutePath self) => Path.GetPathRoot(self); 
	public static bool IsRoot(this AbsolutePath self) => self.GetRoot() == self;

	public static string RelativeToBase(this AbsolutePath self, AbsolutePath root) =>
		Path.GetRelativePath(root, self).WithUnixSeparator();
	
	public static string BaseRelativeTo(this AbsolutePath self, AbsolutePath subfolder) =>
		Path.GetRelativePath(self, subfolder).WithUnixSeparator();
	
	public static bool FileExists(this AbsolutePath path) => File.Exists(path);
	public static bool DirectoryExists(this AbsolutePath path) => Directory.Exists(path);

	public static AbsolutePath ExistingFile(this AbsolutePath path) => path.FileExists() ? path : null;
	public static AbsolutePath ExistingDirectory(this AbsolutePath path) => path.DirectoryExists() ? path : null;

	public static IEnumerable<AbsolutePath> Files(this AbsolutePath path, string pattern = "*", SearchOption searchOption = SearchOption.TopDirectoryOnly)
		=> Directory.EnumerateFiles(path, pattern, searchOption)
			.Select(p => p.AsPath());
	
	public static IEnumerable<AbsolutePath> Directories(this AbsolutePath path, string pattern = "*", SearchOption searchOption = SearchOption.TopDirectoryOnly)
		=> Directory.EnumerateDirectories(path, pattern, searchOption)
			.Select(p => p.AsPath());

	public static bool HasExtension(this AbsolutePath path, string extension, params string[] alternativeExtensions)
		=> alternativeExtensions.Append(extension)
			.Any(e => path.Extension.Equals(e, StringComparison.InvariantCultureIgnoreCase));
	
	public static AbsolutePath WithExtension(this AbsolutePath path, string extension)
		=> path.Parent / Path.ChangeExtension(path.Name, extension);

	public static AbsolutePath CreateDirectory(this AbsolutePath path)
	{
		if (!path.DirectoryExists())
			Directory.CreateDirectory(path);
		return path;
	}

	private static List<AbsolutePath> FileSystemTask(
		Action<string, string> task,
		AbsolutePath path,
		AbsolutePath to,
		string pattern = "*"
	) {
		var output = new List<AbsolutePath>();
		if (path.FileExists())
		{
			to.Parent.CreateDirectory();
			task(path, to);
			output.Add(to);
		}
		else if (path.DirectoryExists())
		{
			foreach (var file in path.Files(pattern, SearchOption.AllDirectories))
			{
				var dst = to / file.RelativeToBase(path);
				output.AddRange(FileSystemTask(task, file, dst));
			}
		}
		return output;
	}

	public static List<AbsolutePath> Copy(this AbsolutePath path, AbsolutePath to, string pattern = "*")
		=> FileSystemTask(
			(src, dst) => File.Copy(src, dst, true),
			path, to, pattern
		);

	public static List<AbsolutePath> CopyInto(this AbsolutePath path, AbsolutePath intoDirectory)
		=> path.Copy(intoDirectory / path.Name);

	public static List<AbsolutePath> Move(this AbsolutePath path, AbsolutePath to, string pattern = "*")
		=> FileSystemTask(
			(src, dst) => File.Move(src, dst, true),
			path, to, pattern
		);

	public static List<AbsolutePath> MoveInto(this AbsolutePath path, AbsolutePath intoDirectory)
		=> path.Move(intoDirectory / path.Name);
}

public static class PathUtils
{
	public static AbsolutePath AsPath(this string input) => AbsolutePath.Create(input);
	public static AbsolutePath AsPath(this FileReference input) => AbsolutePath.Create(input.FullName);
	public static AbsolutePath AsPath(this DirectoryReference input) => AbsolutePath.Create(input.FullName);
	
	internal const char WinSeparator = '\\';
	internal const char UncSeparator = '\\';
	internal const char UnixSeparator = '/';
	
	internal static readonly char[] AllSeparators = new [] { WinSeparator, UncSeparator, UnixSeparator };
	
	public static string WithUnixSeparator(this string input) => input.Replace(WinSeparator, UnixSeparator);
	
	private static bool IsSameDirectory(string pathPart)
		=> pathPart?.Length == 1 &&
		   pathPart[index: 0] == '.';

	private static bool IsUpwardsDirectory(string pathPart)
		=> pathPart?.Length == 2 &&
		   pathPart[index: 0] == '.' &&
		   pathPart[index: 1] == '.';
	
	internal static bool IsWinRoot(string root)
		=> root?.Length == 2 &&
		   char.IsLetter(root[index: 0]) &&
		   root[index: 1] == ':';

	internal static bool IsUnixRoot(string root)
		=> root?.Length == 1 &&
		   root[index: 0] == UnixSeparator;

	internal static bool IsUncRoot(string root)
		=> root?.Length >= 3 &&
		   root[index: 0] == UncSeparator &&
		   root[index: 1] == UncSeparator &&
		   root.Skip(count: 2).All(char.IsLetterOrDigit);
	
	private static string GetHeadPart(string str, int count) => new((str ?? string.Empty).Take(count).ToArray());
	
	internal static bool HasUnixRoot(string path) => IsUnixRoot(GetHeadPart(path, count: 1));
	internal static bool HasUncRoot(string path) => IsUncRoot(GetHeadPart(path, count: 3));
	internal static bool HasWinRoot(string path) => IsWinRoot(GetHeadPart(path, count: 2));
	
	public static string GetPathRoot( string path)
	{
		if (path == null)
			return null;

		if (HasUnixRoot(path))
			return GetHeadPart(path, count: 1);

		if (HasWinRoot(path))
			return GetHeadPart(path, count: 2);

		if (HasUncRoot(path))
		{
			var separatorIndex = path.IndexOf(UncSeparator, startIndex: 2);
			return separatorIndex == -1 ? path : GetHeadPart(path, separatorIndex);
		}

		return null;
	}
	
	public static bool HasPathRoot(string path) => GetPathRoot(path) != null;
	
	private static char GetSeparator(string path)
	{
		var root = GetPathRoot(path);
		if (root != null)
		{
			if (IsWinRoot(root))
				return WinSeparator;

			if (IsUncRoot(root))
				return UncSeparator;

			if (IsUnixRoot(root))
				return UnixSeparator;
		}

		return Path.DirectorySeparatorChar;
	}
	
	private static string Trim(string path)
	{
		if (path == null)
			return null;

		return IsUnixRoot(path) // TODO: "//" ?
			? path
			: path.TrimEnd(AllSeparators);
	}

	public static string Combine(string left, string right, char? separator = null)
	{
		left = Trim(left);
		right = Trim(right);

		if (string.IsNullOrWhiteSpace(left))
			return right;
		if (string.IsNullOrWhiteSpace(right))
			return !IsWinRoot(left) ? left : $@"{left}\";

		separator ??= GetSeparator(left);

		if (IsWinRoot(left))
			return $@"{left}\{right}";
		if (IsUnixRoot(left))
			return $"{left}{right}";
		if (IsUncRoot(left))
			return $@"{left}\{right}";

		return $"{left}{separator}{right}";
	}
	
	public static string NormalizePath(string path, char? separator = null)
	{
		path ??= string.Empty;
		separator ??= GetSeparator(path);
		var root = GetPathRoot(path);

		var tail = root == null ? path : path.Substring(root.Length);
		var tailParts = tail.Split(AllSeparators, StringSplitOptions.RemoveEmptyEntries).ToList();
		for (var i = 0; i < tailParts.Count;)
		{
			var part = tailParts[i];
			if (IsUpwardsDirectory(part))
			{
				if (tailParts.Take(i).All(IsUpwardsDirectory))
				{
					i++;
					continue;
				}

				tailParts.RemoveAt(i);
				tailParts.RemoveAt(i - 1);
				i--;
				continue;
			}

			if (IsSameDirectory(part))
			{
				tailParts.RemoveAt(i);
				continue;
			}

			i++;
		}

		return Combine(root, string.Join(separator.Value, tailParts), separator);
	}
}